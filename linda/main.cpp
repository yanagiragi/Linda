#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <tuple>

using namespace std;

typedef tuple<string, string, string> tupleField;

struct inst
{
	int id;
	string type;
	tupleField field;
};
typedef struct inst inst;


std::vector<std::string> split(const std::string &s, char delim) {
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		// elems.push_back(item);
		elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
	}
	return elems;
}

void PrintTupleField(tupleField tupleField)
{
	cout << "field: [" 
		<< get<0>(tupleField) << "], ["
		<< get<1>(tupleField) << "], ["
		<< get<2>(tupleField) << "]" << endl;
}

void PrintInstruction(inst* instruction)
{
	cout << "id: " << instruction->id << "\t";
	cout << "type: " << instruction->type << "\t";
	PrintTupleField(instruction->field);
}

void ParseInstruction(string line, inst* instruction)
{
	vector<string> splitted = split(line, ' ');
	instruction->id = atoi(splitted[0].c_str());
	instruction->type = splitted[1];
	get<0>(instruction->field) = splitted[2];
	get<1>(instruction->field) = splitted[3];
	get<2>(instruction->field) = splitted[4];
}

void ReadInstructions(const char *filename, vector<inst> *instructions)
{
	std::ifstream fp;

	fp.open(filename);

	std::string buffer;
	std::string line;

	while(fp) {

		std::getline(fp, line);
		
		if (0 == line.compare("exit")) {
			break;
		}

		inst instruction;
		ParseInstruction(line, &instruction);
		instructions->push_back(instruction);

	}

	fp.close();
}

void PrintCurrentSpaces(vector<tupleField> tupleSpace, map<string, string> variables, vector<vector<tupleField>> outputInstances, vector<inst> instructionsQueue)
{
	cout << "Tuple Space (" << tupleSpace.size() << "): " << endl;
	for (auto i : tupleSpace) {
		PrintTupleField(i);
	}

	cout << "Variables (" << variables.size() << "): " << endl;
	for(auto const& i : variables) {
		cout << i.first << ": " << i.second << endl;
	}
	
	cout << "Output Instances: " << endl;
	for (int i = 0; i < outputInstances.size(); ++i) {
		cout << "outputInstances" << i + 1 << " (" << outputInstances[i].size() << "): " << endl;
		for (auto j : outputInstances[i]) {
			PrintTupleField(j);
		}
	}

	cout << "Instructions Queue: " << endl;
	for(auto i : instructionsQueue) {
		PrintInstruction(&i);
	}
}

inst InstructionPreprocess(inst instruction, map<string, string> *variables)
{
	tupleField instructionField = instruction.field;

	string instructionFieldFirst = get<0>(instructionField);
	string instructionFieldSecond = get<1>(instructionField);
	string instructionFieldThird = get<2>(instructionField);

	for(auto const& var : *variables) {
		if (0 == var.first.compare(instructionFieldFirst)) {
			instructionFieldFirst = var.second;
		}
		if (0 == var.first.compare(instructionFieldSecond)) {
			instructionFieldSecond = var.second;
		}
		if (0 == var.first.compare(instructionFieldThird)) {
			instructionFieldThird = var.second;
		}
	}
	
	inst newInstruction;
	newInstruction.id = instruction.id;
	newInstruction.type = instruction.type;
	newInstruction.field = make_tuple(instructionFieldFirst, instructionFieldSecond, instructionFieldThird);
	
	return newInstruction;
}

void InstructionOut(inst instruction, vector<tupleField> *tupleSpace, map<string, string> *variables, vector<vector<tupleField>> *outputInstances, vector<inst> *instructionsQueue) 
{
	tupleField instructionField = instruction.field;

	string instructionFieldFirst = get<0>(instructionField);
	string instructionFieldSecond = get<1>(instructionField);
	string instructionFieldThird = get<2>(instructionField);

	tupleSpace->push_back(make_tuple(instructionFieldFirst, instructionFieldSecond, instructionFieldThird));
}

string ParseQuestionFieldName(string field)
{
	return field.substr(1, field.length() - 1);
}

void UpdateVariable(map<string, string> *variables, string name, string value)
{
	auto iter = variables->find(name);

	if(iter != variables->end()){
		(*variables)[name] = value;
	}
	else {
		variables->insert(pair<string, string>(name, value));
	}
}

void InstructionRead (
	inst instruction,
	vector<tupleField> *tupleSpace,
	map<string, string> *variables,
	vector<vector<tupleField>> *outputInstances,
	vector<inst> *instructionsQueue,
	bool isUpdateInstructionsQueue,
	bool isIn=false
) 
{
	string first  = get<0>(instruction.field);
	string second = get<1>(instruction.field);
	string third  = get<2>(instruction.field);

	bool isMatch = false;

	for(int i = 0; i < tupleSpace->size(); ++i) {

		string tupleSpaceFirst = get<0>(tupleSpace->at(i));
		string tupleSpaceSecond = get<1>(tupleSpace->at(i));
		string tupleSpaceThird = get<2>(tupleSpace->at(i));

		isMatch = 
			(first[0] == '?' || 0 == tupleSpaceFirst.compare(first)) && 
			(second[0] == '?' || 0 == tupleSpaceSecond.compare(second)) &&
			(third[0] == '?' || 0 == tupleSpaceThird.compare(third));

		if (isMatch) {
			
			// cout << "Match: [" << first << "," << second << "," << third << "] :" << "[" << tupleSpaceFirst << ", " << tupleSpaceSecond << ", " << tupleSpaceThird << "]" << endl;

			if (first[0] == '?') {
				string variableName = ParseQuestionFieldName(first);
				UpdateVariable(variables, variableName, tupleSpaceFirst);
			}

			if (second[0] == '?') {
				string variableName = ParseQuestionFieldName(second);
				UpdateVariable(variables, variableName, tupleSpaceSecond);
			}

			if (third[0] == '?') {
				string variableName = ParseQuestionFieldName(third);
				UpdateVariable(variables, variableName, tupleSpaceThird);
			}

			outputInstances->at(instruction.id - 1).push_back(tupleSpace->at(i));

			if (isIn) {
				tupleSpace->erase(tupleSpace->begin() + i);
			}

			if (isUpdateInstructionsQueue == false) {
				
				// currently dealing instruction queue, remember to remove matched instruction in instruction queue

				struct findInstruction : std::unary_function<inst, bool> {
					inst m_inst;
					findInstruction(inst i): m_inst(i) { }
					bool operator()(inst const& i) const {
						return i.id == m_inst.id &&
							i.type == m_inst.type &&
							get<0>(i.field) == get<0>(m_inst.field) && 
							get<1>(i.field) == get<1>(m_inst.field) && 
							get<2>(i.field) == get<2>(m_inst.field)
						;
					}
				};

				auto iter = find_if(instructionsQueue->begin(), instructionsQueue->end(), findInstruction(instruction));
				instructionsQueue->erase(iter);
			}

			break;
		}
	}
	
	if (!isMatch) {
		if (isUpdateInstructionsQueue) {
			instructionsQueue->push_back(instruction);
		}
	}
}

void InstructionIn(
	inst instruction,
	vector<tupleField> *tupleSpace,
	map<string, string> *variables,
	vector<vector<tupleField>> *outputInstances,
	vector<inst> *instructionsQueue,
	bool isUpdateInstructionsQueue) 
{
	InstructionRead(instruction, tupleSpace, variables, outputInstances, instructionsQueue, isUpdateInstructionsQueue, true);
}

void DealInstructions(
	vector<inst> *instructions,
	vector<tupleField> *tupleSpace,
	map<string, string> *variables,
	vector<vector<tupleField>> *outputInstances,
	vector<inst> *instructionsQueue,
	bool isUpdateInstructionsQueue,
	bool verbose=false) 
{
	// Deal Instructions
	for(auto instruction : *instructions) {

		auto preProcessedInstruction = InstructionPreprocess(instruction, variables);

		if (0 == instruction.type.compare("out")) {
			InstructionOut(preProcessedInstruction, tupleSpace, variables, outputInstances, instructionsQueue);
			DealInstructions(instructionsQueue, tupleSpace, variables, outputInstances, instructionsQueue, false, false);
		}
		else if (0 == instruction.type.compare("in")) {
			InstructionIn(preProcessedInstruction, tupleSpace, variables, outputInstances, instructionsQueue, isUpdateInstructionsQueue);
		}
		else if (0 == instruction.type.compare("read")) {
			InstructionRead(preProcessedInstruction, tupleSpace, variables, outputInstances, instructionsQueue, isUpdateInstructionsQueue);
		}
		else {
			cout << "Error: ";
			PrintInstruction(&instruction);
			cout << endl;
		}

		if (verbose) {		
			cout << instruction.id << ", " << instruction.type << ", " << get<0>(instruction.field) << "," << get<1>(instruction.field) << "," << get<2>(instruction.field) << endl;
			PrintCurrentSpaces(*tupleSpace, *variables, *outputInstances, *instructionsQueue);
			cout << endl;
		}
	}
}

int main(int argc, char** argv)
{

	vector<inst> instructions;

	ReadInstructions("in.txt", &instructions);

	// For Debug
	// for( auto i : instructions ) { PrintInstruction(&i); }
	
	vector<tupleField> tupleSpace;
	map<string, string> variables;
	vector<vector<tupleField>> outputInstances;
	vector<inst> instructionsQueue;

	// Initial outputInstances
	int instancesCount = 0;
	for(auto instruction : instructions) {
		if(instancesCount < instruction.id) {
			instancesCount = instruction.id; 
		}
	}

	for(int i = 0; i < instancesCount; ++i) {
		outputInstances.push_back(vector<tupleField>());
	}

	bool verbose = false;
	bool isUpdateInstructionsQueue = true;
	DealInstructions(&instructions, &tupleSpace, &variables, &outputInstances, &instructionsQueue, isUpdateInstructionsQueue, verbose);
	
	cout << "Output Instances: " << endl;
	for (int i = 0; i < outputInstances.size(); ++i) {
		cout << "outputInstances" << i + 1 << " (" << outputInstances[i].size() << "): " << endl;
		for (auto j : outputInstances[i]) {
			PrintTupleField(j);
		}
	}

	return 0;
}
