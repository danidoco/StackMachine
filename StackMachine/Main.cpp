#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>

constexpr int STACK_SIZE = 100;
constexpr int PROGRAM_SIZE = 100;
constexpr char SOURCE_PATH[] = "Example";

enum class Opcode : unsigned char
{
	Quit,
	Nothing,
	Push,
	Drop,
	Duplicate,
	Over,
	Dot,
	Emit,
	Add,
	Substract,
	Multiply,
	Equal,
	Greater,
	Lesser,
	Not,
	Then,
	Goto,
	_Default,
};

struct TokenMap
{
	const char* token;
	Opcode op;
};

struct Inst
{
	Opcode op;
	int literal;
};

static bool IsInt(const std::string& str)
{
	if (str.empty())
	{
		return false;
	}

	size_t i = 0;

	if (str[0] == '-' || str[0] == '+')
	{
		i = 1;
	}

	if (i == str.size())
	{
		return false;
	}

	for (; i < str.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(str[i])))
		{
			return false;
		}
	}

	return true;
}

static bool IsLabelDecl(const std::string& str)
{
	if (str.size() < 2)
	{
		return false;
	}

	if (str.back() != ':')
	{
		return false;
	}

	for (size_t i = 0; i < str.size() - 1; ++i)
	{
		unsigned char c = static_cast<unsigned char>(str[i]);
		if (!std::isalnum(c) && c != '_')
		{
			return false;
		}
	}

	return true;
}

static bool IsLabelRef(const std::string& str)
{
	if (str.size() < 2)
	{
		return false;
	}

	if (str.front() != ':') 
	{
		return false;
	}

	for (size_t i = 1; i < str.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(str[i]);
		if (!std::isalnum(c) && c != '_') 
		{
			return false;
		}
	}

	return true;
}

static Opcode ParseOpcode(const std::string& token)
{
	const TokenMap tokenMap[] =
	{
		{"quit", Opcode::Quit},
		{"nop", Opcode::Nothing},
		{"drop", Opcode::Drop},
		{"dup", Opcode::Duplicate},
		{"over", Opcode::Over},
		{".", Opcode::Dot},
		{".c", Opcode::Emit},
		{"+", Opcode::Add},
		{"-", Opcode::Substract},
		{"*", Opcode::Multiply},
		{"==", Opcode::Equal},
		{">", Opcode::Greater},
		{"<", Opcode::Lesser},
		{"!", Opcode::Not},
		{"then", Opcode::Then},
		{"goto", Opcode::Goto}
	};

	for (size_t i = 0; i < sizeof(tokenMap) / sizeof(tokenMap[0]); ++i)
	{
		if (token == tokenMap[i].token)
		{
			return tokenMap[i].op;
		}
	}

	return Opcode::_Default;
}

int main()
{
	// load source file
	std::ifstream file(SOURCE_PATH);
	if (!file.is_open()) return 1;
	std::vector<std::string> tokens;
	std::string token;
	while (file >> token) tokens.push_back(token);

	// preprocessing
	std::unordered_map<std::string, unsigned int> labels;
	for (unsigned int i = 0; i < tokens.size(); i++)
	{
		std::string& t = tokens[i];
		
		if (IsLabelDecl(t))
		{
			std::string name = t.substr(0, t.size() - 1);
			labels[name] = i;
			t = "nop";
		}
	}
	for (unsigned int i = 0; i < tokens.size(); i++)
	{
		std::string& t = tokens[i];

		if (IsLabelRef(t))
		{
			std::string name = t.substr(1);
			auto it = labels.find(name);
			if (it == labels.end())
			{
				return 69;
			}
			t = std::to_string(it->second);
		}
	}

	// parse tokens into bytecode
	Inst program[PROGRAM_SIZE] = { Opcode::Quit };
	for (unsigned int i = 0; i < tokens.size(); i++)
	{
		const std::string& t = tokens[i];

		if (IsInt(t))
		{
			program[i] = { Opcode::Push, std::stoi(t) };

		}
		else
		{
			program[i] = { ParseOpcode(t), 0 };
		}
	}

	// runtime environment
	int pc = 0;
	int sp = 0;
	int reg = 0;
	int* stack = (int*)std::malloc(sizeof(int) * STACK_SIZE);
	if (!stack) return 1;

	// execute bytecode
	bool running = true;
	while (pc < PROGRAM_SIZE && running)
	{
		Inst inst = program[pc];

		switch (inst.op)
		{
		case Opcode::Quit:
			running = false;
			break;

		case Opcode::Nothing:
			break;

		case Opcode::Push:
			stack[sp] = inst.literal;
			sp++;
			break;

		case Opcode::Drop:
			sp--;
			break;

		case Opcode::Duplicate:
			reg = stack[sp - 1];
			stack[sp] = reg;
			sp++;
			break;

		case Opcode::Over:
			reg = stack[sp - 2];
			stack[sp] = reg;
			sp++;
			break;

		case Opcode::Dot:
			sp--;
			std::printf("%d", stack[sp]);
			break;

		case Opcode::Emit:
			sp--;
			putchar(stack[sp]);
			break;

		case Opcode::Add:
			reg = stack[sp - 1];
			sp--;
			reg += stack[sp - 1];
			sp--;
			stack[sp] = reg;
			sp++;
			break;

		case Opcode::Substract:
			reg = stack[sp - 1];
			sp--;
			reg -= stack[sp - 1];
			sp--;
			stack[sp] = reg;
			sp++;
			break;

		case Opcode::Multiply:
			reg = stack[sp - 1];
			sp--;
			reg *= stack[sp - 1];
			sp--;
			stack[sp] = reg;
			sp++;
			break;

		case Opcode::Equal:
			reg = stack[sp - 1];
			sp--;
			reg -= stack[sp - 1];
			sp--;
			stack[sp] = (reg == 0) ? 1 : 0;
			sp++;
			break;

		case Opcode::Greater:
			reg = stack[sp - 1];
			sp--;
			reg -= stack[sp - 1];
			sp--;
			stack[sp] = (reg < 0) ? 1 : 0;
			sp++;
			break;
		
		case Opcode::Lesser:
			reg = stack[sp - 1];
			sp--;
			reg -= stack[sp - 1];
			sp--;
			stack[sp] = (reg > 0) ? 1 : 0;
			sp++;
			break;

		case Opcode::Not:
			reg = stack[sp - 1];
			stack[sp - 1] = (reg == 0) ? 1 : 0;
			break;

		case Opcode::Then:
			reg = stack[sp - 1];
			sp--;
			if (stack[sp - 1]) pc = reg - 1;
			sp--;
			break;
		
		case Opcode::Goto:
			reg = stack[sp - 1];
			sp--;
			pc = reg - 1;
			break;

		default:
			__debugbreak();
			break;
		}

		pc++;
	}
	
	std::free(stack);
	return 0;
}
