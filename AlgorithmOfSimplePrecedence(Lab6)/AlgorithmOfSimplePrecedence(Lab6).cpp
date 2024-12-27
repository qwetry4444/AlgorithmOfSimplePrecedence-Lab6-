#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string>
#include <stack>
#include <unordered_map>
#include <bitset>


typedef enum Relation
{
	None = ' ', Before = '<', Together = '=', After = '>', Dual = '%'
} Relation;

enum Action
{
	Shift, Reduce, Stop, Error
};

struct Symbol
{
	char symbol;
	Relation rel;
	std::string id;
	unsigned value;
	int triadNumber = 0;
	// Добавить поле номера триады
	Symbol(char _symbol, Relation _rel, std::string _id, unsigned _value) : symbol(_symbol), rel(_rel), id(_id), value(_value) {}
	Symbol() : symbol('\0'), rel(Relation{}), id(""), value(0) {}
};

std::stack<Symbol> stack;

std::string Native = "=;|&~()";
const char alpabet[] = "LSETM=;|&~()IC#";

FILE* fi, * foScanner, * foTriads;
int cc = 0;
Symbol c;


typedef struct Rule
{
	char left;
	std::string right;
} Rule;

Rule rules[] =
{
	{'_', "#L#"},
	{'L', "LS"}, {'L', "S"},
	{'S', "I=E;"},
	{'E', "E|T"}, {'E', "T"},
	{'T', "T&M"}, {'M', "~M"}, {'T', "M"},
	{'M', "(E)"}, {'M', "I"}, {'M', "C"}
};

Relation matrix[15][15] = 
{ 
	{None, Together, None, None, None, None, None, None, None, None, None, None, Before, None, Together },
	{None, None, None, None, None, None, None, None, None, None, None, None, After, None, After },
	{None, None, None, None, None, None, Together, Together, None, None, None, Together, None, None, None },
	{None, None, None, None, None, None, After, After, Together, None, None, After, None, None, None },
	{None, None, None, None, None, None, After, After, After, None, None, After, None, None, None },
	{None, None, Dual, Before, Before, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, None, None, None, None, None, None, None, None, After, None, After },
	{None, None, None, Together, Before, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, Together, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, Together, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, Dual, Before, Before, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, None, None, After, After, After, None, None, After, None, None, None },
	{None, None, None, None, None, Together, After, After, After, None, None, After, None, None, None },
	{None, None, None, None, None, None, After, After, After, None, None, After, None, None, None },
	{Dual, Before, None, None, None, None, None, None, None, None, None, None, Before, None, None }
};


int triadNumber = 1;
struct Triad {
	int number;
	char operation;
	std::string operand1;
	std::string operand2;

	Triad(char _operation, std::string _operand1, std::string _operand2)
		: number(triadNumber), operation(_operation), operand1(_operand1), operand2(_operand2) { }

	Triad() : number(triadNumber) {}
};

std::unordered_map<std::string, unsigned> variables;
std::unordered_map<int, Triad> triads;

void addTriad(Triad triad) {
	triads[triad.number] = triad;
	triadNumber++;
}

void OnError(const char* msg, const char* param);
void fprintfTriads();
void fprintfTriad(Triad triad);
std::string toBinary(unsigned number);
void OpenFile(FILE*& file, const char* fileName, const char* mode, const char* errorMessage);


void Get(void)
{
	cc = fgetc(fi);
}

void GetLex(void)
{
	c = Symbol();

	while (isspace(cc)) {
		fprintf(foScanner, "%c", cc);
		Get();
	}

	if (isalpha(cc) || cc == '_')
	{
		c.id = cc;
		Get();
		while (cc == '0' || cc == '1')
		{
			c.id += cc;
			Get();
		}
		c.symbol = 'I';
		//addTriad(Triad('V', c.id, "¤"));
		//Добавить переменную в список перменных
	}
	else if (cc == '0' || cc == '1')
	{
		std::string buffer;
		buffer = cc;
		Get();
		while (cc == '0' || cc == '1')
		{
			buffer += cc;
			Get();
		}
		
		//addTriad(Triad('C', buffer, "¤"));
		c.symbol = 'C';
		c.value = std::stoi(buffer, nullptr, 2);
	}
	else if (Native.find(cc) != std::string::npos)
	{
		c.symbol = cc;
		Get();
	}
	else if (cc == EOF)
		c.symbol = '#';
	else
		OnError("Unknown symbol\n", NULL);
	fprintf(foScanner, "%c", c.symbol);
}


int charIndex(char c)
{
	const char* t;
	t = strchr(alpabet, c);
	if (t)
		return t - alpabet;
	else
		OnError("Unknown symbol", NULL);
}

Relation getRelation(char x, char y)
{
	return matrix[charIndex(x)][charIndex(y)];
}


void OnReduce(int ruleNumber) {
	if (ruleNumber < 0 || ruleNumber >= std::size(rules)) {
		OnError("Invalid rule number", nullptr);
		return;
	}

	Symbol newSymbol;
	Symbol t, e, i, m;
	Triad triad;

	switch (ruleNumber) {
	case 0:
		return;

	case 1:  // L -> LS
		newSymbol.symbol = 'L';
		stack.pop(); // Удаляем S
		stack.pop(); // Удаляем L
		break;

	case 2:  // L -> S
		newSymbol = stack.top();
		newSymbol.symbol = 'L';
		stack.pop(); // Удаляем S
		break;

	case 3:  // S -> I=E;
		newSymbol.symbol = 'S';
		stack.pop(); // Удаляем ';'
		e = stack.top();
		stack.pop(); // Удаляем E
		stack.pop(); // Удаляем '='
		i = stack.top();
		stack.pop(); // Удаляем I

		if (variables.count(i.id) != 0) {
			variables[i.id] = e.value;
		}

		newSymbol.id = i.id;
		newSymbol.value = e.value;

		triad = Triad();
		triad.operation = '=';
		triad.operand1 = i.id;
		triad.operand2 = !e.id.empty() ? e.id : toBinary(e.value);
		addTriad(triad);
		break;

	case 4:  // E -> E|T
		newSymbol.symbol = 'E';
		t = stack.top();
		stack.pop(); // Удаляем T
		stack.pop(); // Удаляем '|'
		e = stack.top();
		stack.pop(); // Удаляем E

		newSymbol.value = t.value | e.value;

		triad = Triad('|',
			e.id.empty() ? toBinary(e.value) : e.id,
			t.id.empty() ? toBinary(t.value) : t.id);
		addTriad(triad);
		break;

	case 5:  // E -> T
		newSymbol.symbol = 'E';
		t = stack.top();
		if (!t.id.empty()) newSymbol.id = t.id;
		if (t.id.empty()) newSymbol.value = t.value;
		stack.pop(); // Удаляем T
		break;

	case 6:  // T -> T&M
		newSymbol.symbol = 'T';
		m = stack.top();
		stack.pop(); // Удаляем M
		stack.pop(); // Удаляем '&'
		t = stack.top();
		stack.pop(); // Удаляем T

		newSymbol.value = m.value & t.value;

		triad = Triad('&',
			t.id.empty() ? toBinary(t.value) : t.id,
			m.id.empty() ? toBinary(m.value) : m.id);
		addTriad(triad);
		break;

	case 7:  // M -> ~M
		newSymbol.symbol = 'M';
		m = stack.top();
		stack.pop(); // Удаляем M
		stack.pop(); // Удаляем '~'

		newSymbol.value = ~m.value;

		triad = Triad('~',
			m.id.empty() ? toBinary(m.value) : m.id,
			"¤");
		addTriad(triad);
		break;

	case 8:  // T -> M
		newSymbol.symbol = 'T';
		m = stack.top();
		newSymbol.value = m.value;
		stack.pop(); // Удаляем M
		break;

	case 9:  // M -> (E)
		newSymbol.symbol = 'M';
		stack.pop(); // Удаляем ')'
		e = stack.top();
		stack.pop(); // Удаляем E
		stack.pop(); // Удаляем '('
		newSymbol.value = e.value;
		break;

	case 10: // M -> I
		newSymbol = stack.top();
		newSymbol.symbol = 'M';
		stack.pop(); // Удаляем I или C
		addTriad(Triad('V', c.id, "¤"));
	case 11: // M -> C
		newSymbol = stack.top();
		newSymbol.symbol = 'M';
		stack.pop(); // Удаляем I или C
		addTriad(Triad('C', toBinary(c.value), "¤"));
		break;

	default:
		OnError("Wrong rule number", nullptr);
		return;
	}
	
	if (!stack.empty()) {
		newSymbol.rel = getRelation(stack.top().symbol, newSymbol.symbol);
	}
	else {
		newSymbol.rel = None;
	}
	stack.push(newSymbol);
}

int g() {
	std::string omega;
	int currentRuleNumber = -1;
	std::stack<Symbol> tempStack = stack;
	Symbol stackTop = tempStack.top();
	
	do {
		omega = tempStack.top().symbol + omega;
		if (stackTop.rel == Dual || stackTop.rel == Before) {
			for (int i = 0; i < std::size(rules); i++) {
				if (omega == rules[i].right) {
					currentRuleNumber = i;
				}
			}
		}

		tempStack.pop();
	} while (!tempStack.empty() && stackTop.rel != Before);
		
	return currentRuleNumber;
	//Rule not for all 
	//std::stack<Symbol> tempStack = stack;
	//while (!tempStack.empty()) {
	//	omega = tempStack.top().symbol + omega;
	//	tempStack.pop();
	//}
	// Последовательное считывание элементов с вершины
	// стека при обнаружении <= сохранить метку, продолжить считывание 
	// при выходе за макс размер правила вернуться в сохраненной метке

	//for (int i = 0; i < std::size(rules); ++i) {
	//	if (omega.size() >= rules[i].right.size() &&
	//		omega.substr(omega.size() - rules[i].right.size()) == rules[i].right) {
	//		return i;
	//	}
	//}
}

Action f() {
	if (stack.top().symbol == 'L' && c.symbol == '#') {
		return Stop;
	}
	if (c.rel == None) {
		return Action::Error;
	}
	if (c.rel == Before || c.rel == Together || c.rel == Dual) {
		return Shift;
	}
	if (c.rel == After) {
		return Action::Reduce;
	}
}

void ProcessAction(Action currentAction, bool& skipNextGetLex, bool& exitLoop, int& ruleNumber) {
	switch (currentAction) {
	case Action::Shift:
		stack.push(c);
		break;

	case Action::Reduce:
		ruleNumber = g();
		if (ruleNumber == -1) {
			OnError("No rule", NULL);
		}
		OnReduce(ruleNumber);
		c.rel = getRelation(stack.top().symbol, c.symbol);
		skipNextGetLex = true;
		break;

	case Action::Stop:
		exitLoop = true;
		break;

	case Action::Error:
		OnError("No relation", NULL);
		break;
	}
}

int main()
{
	OpenFile(fi, "test1.txt", "rb", "Input file open error.");
	OpenFile(foScanner, "test1_resultScanner.txt", "wb", "Output file open error.");
	OpenFile(foTriads, "test1_resultTriads.txt", "wb", "Output file open error.");

	Action currentAction;
	int ruleNumber = -1;
	bool skipNextGetLex = false;
	bool exitLoop = false;

	stack.push(Symbol('#', None, "", 0));
	Get();

	do {
		if (!skipNextGetLex) {
			GetLex();
			c.rel = getRelation(stack.top().symbol, c.symbol);
		}
		skipNextGetLex = false;
		currentAction = f();

		ProcessAction(currentAction, skipNextGetLex, exitLoop, ruleNumber);
	} while (!exitLoop);
	fprintfTriads();
}


void OpenFile(FILE*& file, const char* fileName, const char* mode, const char* errorMessage) {
	file = fopen(fileName, mode);
	if (!file) {
		throw std::runtime_error(errorMessage);
	}
}

std::string toBinary(unsigned number) {
	int bitCount = (number == 0) ? 1 : static_cast<int>(std::log2(number)) + 1;
	return std::bitset<32>(number).to_string().substr(32 - bitCount);
}

void fprintfTriad(Triad triad) {
	fprintf(foTriads, "%d:\t%c(%s, %s)\n", triad.number, triad.operation, triad.operand1.c_str(), triad.operand2.c_str());
}

void fprintfTriads() {
	for (const auto& [key, value] : triads) {
		fprintfTriad(value);
	}
}

void OnError(const char* msg, const char* param)
{
	printf("\nError: ");
	printf(msg);
	printf("\n");
	fclose(fi);
	fclose(foTriads);
	fclose(foScanner);
	exit(7);
}