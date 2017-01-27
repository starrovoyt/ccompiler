#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <stdio.h>
#include <vector>
#include <stack>
#include <cmath>
using namespace std;

char c;
ifstream in;
ofstream out;

struct pcell //polis cell
{
    long type = - 1;
    long cint = 0;
    double cdouble = 0;
    string str = "";
    long line = 0;
};

vector <pcell> polis;
vector <pcell> lex;
int line = 1;
long lexc = 0;
bool unmin = false;
long k = 0;

void printPolis();

struct opcell
{
    string op;
    long line;
};

void error(string problem, long line)
{
    if (problem != "")
        cout << problem << " в строке " << line << endl;
    //printPolis();
    exit(0);
}

void createPcell(string lexem, string token)
{
    pcell p;
    p.cint = 0;
    p.cdouble = 0;
    p.line = line;
    if (token == "string")
    {
        p.type = 5;
        p.str = lexem;
    }
    if (token == "id")
    {
        p.type = 6;
        p.str = lexem;
    }
    if (token == "operation" || lexem == ")" || lexem == "(")
    {
        p.type = 1;
        p.str = lexem;
    }
    if (token == "reserved word" || token == "punctuation" || lexem == "<<")
    {
        p.type = 7; //ненужный тип
        p.str = lexem;
    }
    if (lexem == "endl" && token == "reserved word")
    {
        p.type = 5;
        p.str = "\n";
    }
    if (lexem == ")" || lexem == "(")
    {
        p.type = 1;
        p.str = lexem;
    }
    if (lexem == "true" || lexem == "false")
    {
        p.type = 2;
        if (lexem == "true")
            p.cint = 1;
        else p.cint = 0;
    }
    if (token == "const")
    {
        bool neg = (lexem[0] == '-');
        int i = 0, a = 0, b = 0;
        if (neg) i++;
        while (i < lexem.size() && lexem[i] != '.')
            a = 10 * a + int(lexem[i++]) - int('0');
        i++;
        if (i < lexem.size())
        {
            int tendeg = 1;
            while (i < lexem.size())
            {
                b = b*10 + int(lexem[i++]) - int('0');
                tendeg = tendeg*10;
            }
            p.cdouble = a + (double) b/tendeg;
            if (neg) p.cdouble = -p.cdouble;
            p.type = 4;
        }
        else
        {
            p.cint = a;
            if (neg) p.cint = -p.cint;
            p.type = 3;
        }
    }
    
    lex.push_back(p);
}

// ЛЕКСИЧЕСКИЙ

bool boolAnalyser(string & s, string & token);

bool isReservedWord(string s)
{
    if (s == "cinout" || s == "for" || s == "int" || s == "double" || s == "bool" || s == "to" || s == "else" || s == "while" || s == "do" || s == "main" || s == "true" || s == "false" || s == "downto" || s == "endl" || s == "if")
        return true;
    
    return false;
}

bool isNumber(string & s, string & token)
{
    bool point = false;
    
    while ( (( c == '.' && !point ) || ( c <= '9' && c >= '0' )) && !in.eof() )
    {
        if (c == '.')
            point = true;
        s += c;
        in.get(c);
    }
    token = "const";
    
    //если точка и уже была до нее ЛИБО если точка - последний символ и дробная часть отсутствует
    if ((c == '.')&&(point))
        error("ERROR in isNumber: вторая точка или отсутствие дробной части в const типа double",line);
    
    unmin = false;
    return true;
}

void skipComment()
{
    if (c == '/')
    {
        while (c != '\n' && !in.eof())
            in.get(c);
    }
    else
    {
        while (!in.eof())
        {
            in.get(c);
            if (c == '*')
            {
                in.get(c);
                if (c == '/')
                {
                    in.get(c);
                    break;
                };
            }
        }
    }
    
    while (!in.eof() && (c == ' ' || c == '\n' || c == '\0'))
    {
        if (c == '\n')
            line++;
        in.get(c);
    }
    
}

bool isWord(string & s, string & token) //перевести в нижний регистр
{
    while ((( c >= 'a' && c <= 'z' ) || ( c <= '9' && c >= '0' ) || ( c >= 'A' && c <= 'Z' ) || c == '_') && !in.eof() )
    {
        s += c;
        in.get(c);
    }
    
    if (isReservedWord(s))
        token = "reserved word";
    else token = "id";
    
    unmin = false;
    
    return true;
}

bool isOperation(string & s, string & token)
{
    s += c;
    token = "operation";
    
    if ( (c == '<' || c == '>') && !in.eof() )
    {
        in.get(c);
        if (c == '=' || c == s[0])
        {
            s += c;
            if (!in.eof())
                in.get(c);
        }
        unmin = true;
        return true;
    }
    
    if ( (c == '=' || c == '!') && !in.eof() )
    {
        if (c == '=') unmin = true;
        
        if (in.eof()) error("ERROR in isOperation: неожиданный конец программы",line);
        
        in.get(c);
        if (c == '=')
        {
            s += c;
            if (!in.eof())
                in.get(c);
        }
        return true;
    }
    
    if ( (c == '+' || c == '-') && !in.eof() )
    {
        if (in.eof()) error("ERROR in isOperation: неожиданный конец программы",line);
        
        in.get(c);
        
        if (unmin) //если минус/плюс должны быть унарными
        {
            while (!in.eof() && c == ' ')
                in.get(c);
            
            if (in.eof())
                error("ERROR in isOperation: неожиданный конец программы",line);
            
            if (c >= '0' && c <= '9')
                return isNumber(s, token);
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '(')
            {
                if (s[0] == '+')
                    return boolAnalyser(s, token);
            }
            else return false;
        }
        
        if (s[0] == c) //разбор инкремент/не инкремент
        {
            char saved = c;
            if (in.eof())
                error("ERROR in isOperation: неожиданный конец программы",line);
            
            in.get(c);
            
            while (!in.eof() && c == ' ')
                in.get(c);
            
            
            if (in.eof())
                error("ERROR in isOperation: неожиданный конец программы",line);
            
            if ((c>='0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '(')
            {
                //тогда это унарный плюс/минус
                // дело в том, что два минуса подряд дадут плюс и два плюса подряд дадут плюс, тогда
                s = "+";
            }
            else s += saved;
            return true;
        }
        unmin = true;
        return true;
    }
    
    if (c == '|' || c == '&')
    {
        if (in.eof())
            error("ERROR in isOperation: неожиданный конец программы",line);
        
        in.get(c);
        if (c == s[0])
        {
            s += c;
            if (!in.eof())
                in.get(c);
            return true;
        }
        return false;
        
    }
    
    if (!in.eof())
        in.get(c);
    
    return true;
}

bool boolAnalyser(string & s, string & token)
{
    while (!in.eof() && (c == ' ' || c == '\n' || c == '\0'))
    {
        if (c == '\n')
            line++;
        in.get(c);
    }
    
    if (c == '/')
    {
        if (in.eof())
            error("ERROR in boolAnalyser: неожиданный конец программы",line);
        
        in.get(c);
        if (c == '/' || c == '*')
        {
            skipComment();
            if (!in.eof())
                return boolAnalyser(s, token);
            else return false;
        }
        else
        {
            token = "operation";
            s = "/";
            return true;
        }
    }
    
    //разбор случаев
    if ((c <= '9')&&(c >= '0'))
        return isNumber(s,token);
    
    if (((c >= 'a')&&(c <= 'z')) || ((c >= 'A')&&(c <= 'Z')) || (c == '_'))
        return isWord(s,token);
    
    if ( c == '<' || c == '>' || c == '=' || c == '!' || c == '+' || c == '-' ||  c == '%' || c == '^' || c == '|' || c == '&' || c == '*' || c == '<' || c == '>')
        return isOperation(s,token);
    
    if (c == ')' || c == '(' || c == '{' || c == '}' || c == ';' || c == ',')
    {
        s="";
        if (c == '(') unmin = true;
        s += c;
        token = "punctuation";
        in.get(c);
        return true;
    }
    
    //оператор присваивания для pfor
    if (c == ':')
    {
        in.get(c);
        if (c != '=')
            return false;
        s = ":=";
        token = "operation";
        unmin = true;
        in.get(c);
        return true;
    }
    
    //строка в кавычках
    if (c == '"')
    {
        if (in.eof())
            error("ERROR in boolAnalyser: неожиданный конец программы",line);
        
        in.get(c);
        
        while (!in.eof() && c != '"')
        {
            s += c;
            in.get(c);
        }
        
        if (in.eof())
            error("ERROR in boolAnalyser: неожиданный конец программы",line);
        
        in.get(c);
        
        token = "string";
        unmin = false;
        return true;
    }
    
    error("ERROR in boolAnalyser: неожиданный символ в строке",line);
    return false;
}

void lexicalAnalyser(string & s, string & token)
{
    s = ""; token = "";
    if (!boolAnalyser(s, token))
        error("",line);
    createPcell(s, token);
}

// СИНТАКСИЧЕСКИЙ

string s, token;
bool fl;

bool isName()
{
    if (token != "id")
        return false;
    
    lexicalAnalyser(s, token);
    return true;
}

bool isBoolValue()
{
    if (s == "true" || s == "false")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isConst1() // проверка на константу без считывания
{
    if (token != "const")
        return false;
    
    return true;
}

bool isConst() // проверка на константу с последующим считыванием
{
    if (token != "const")
        return false;
    
    lexicalAnalyser(s, token);
    return true;
}

bool isInc()
{
    if (s == "++" || s == "--")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isAssignment()
{
    if (s == "=")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isDeg()
{
    if (s == "^")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isMultiply()
{
    if (s == "*" || s == "/" || s == "&&" || s == "div" || s == "%")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isAddition()
{
    if (s == "+" || s == "||" || s == "-")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isExpression();

bool isAtom();

bool isSpecAtom()
{
    if (s == "true" || s == "false")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    if (s == "!")
    {
        lexicalAnalyser(s, token);
        return isAtom();
    }
    if (s == "-")
    {
        lexicalAnalyser(s, token);
        return isAtom();
    }
    return isConst();
}

bool isAtom()
{
    if (token == "id")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    if (s == "(")
    {
        lexicalAnalyser(s, token);
        if (isExpression())
        {
            if (s == ")")
            {
                lexicalAnalyser(s, token);
                return true;
            }
            else return false;
        }
        else return false;
    }
    if (isSpecAtom())
    {
        return true;
    }
    return (isExpression());
    return false;
}

bool isAtom1()
{
    if (!isAtom())
    {
        return false;
    }
    if (s == "^")
    {
        lexicalAnalyser(s, token);
        return isAtom();
    }
    if (isInc()) return true;
    return true;
}

bool isTerm()
{
    if (!isAtom1()) return false;
    bool fl = true;
    while (fl)
    {
        if (!isMultiply()) break;
        else
        {
            if (!isAtom1()) fl = false;
        }
    }
    return fl;
}

bool isSimpleExpression()
{
    if (!isTerm()) return false;
    bool fl = true;
    while (fl)
    {
        if (!isAddition()) break;
        else
        {
            if (!isTerm()) fl = false;
        }
    }
    return fl;
}

bool isComparing()
{
    if (s == "==" || s == ">" || s == "<" || s == "<=" || s == ">=" || s == "!=")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isExpression1() // чет странненькое
{
    bool fl = false;
    if (isSimpleExpression()) fl = true;
    if (fl)
    {
        if (isComparing())
        {
            if (isExpression1()) return true;
            return false;
        }
    }
    return fl;
}

bool isNameAtom1()
{
    if (s == "^")
    {
        lexicalAnalyser(s, token);
        return isAtom();
    }
    if (s == "++" || s == "--") return isInc();
    return true;
}

bool isNameTerm()
{
    if (!isNameAtom1()) return false;
    bool fl = true;
    while (fl)
    {
        if (!isMultiply()) break;
        else
        {
            if (!isAtom1()) fl = false;
        }
    }
    return fl;
}

bool isNameSimpleExpression()
{
    if (!isNameTerm()) return false;
    bool fl = true;
    while (fl)
    {
        if (!isAddition()) break;
        else
        {
            if (!isTerm()) fl = false;
        }
    }
    return fl;
}

bool isNameExp1()
{
    bool fl = false;
    if (isNameSimpleExpression()) fl = true;
    if (fl)
    {
        if (isComparing())
        {
            if (isExpression1()) return true;
            return false;
        }
    }
    return fl;
}

bool isExpression()
{
    bool con = isConst1();
    if (s == "true" || s == " false" || s == "!" || con || s == "(" || s == "-")
        return isExpression1();
    if (token != "id") return false;
    lexicalAnalyser(s, token);
    if (s == "=")
    {
        lexicalAnalyser(s, token);
        return isExpression();
    }
    else
    {
        return isNameExp1();
    }
}

// ОПЕРАТОР

bool isOperator();

bool isElse()
{
    if (s == "else")
    {
        lexicalAnalyser(s, token);
        if (isOperator())
        {
            return true;
        }
    }
    return false;
}

bool isDirection()
{
    if (s == "to" || s == "downto")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isPfor()
{
    lexicalAnalyser(s, token);
    if (isExpression())
    {
        if (isDirection())
        {
            if (isExpression())
            {
                if (s == ")")
                {
                    lexicalAnalyser(s, token);
                    if (isOperator())
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool isDescription();

bool isCfor()
{
    if (s == ";")
    {
        lexicalAnalyser(s, token);
        if (isExpression())
        {
            if (s == ";")
            {
                lexicalAnalyser(s, token);
                if (isExpression())
                {
                    if (s == ")")
                    {
                        lexicalAnalyser(s, token);
                        if (isOperator()) return true;
                    }
                }
            }
        }
    }
    else
    {
        if (isExpression())
        {
            if (s == ";")
            {
                lexicalAnalyser(s, token);
                if (isExpression())
                {
                    if (s == ")")
                    {
                        lexicalAnalyser(s, token);
                        if (isOperator()) return true;
                    }
                }
            }
        }
    }
    return false;
}

bool isForFor()
{
    if (s == "=")
    {
        lexicalAnalyser(s, token);
        return isExpression();
    }
    else
    {
        return isNameExp1();
    }
}

bool isFor()
{
    if (s != "for") return false;
    lexicalAnalyser(s, token);
    if (s != "(") return false;
    lexicalAnalyser(s, token);
    
    if (s == "bool" || s == "int" || s == "double")
    {
        if (!isDescription()) return false;
        if (!isCfor()) return false;
        if (s == "else")
        {
            return isElse();
        }
        return true;
    }
    
    if (token == "id")
    {
        lexicalAnalyser(s, token);
        if (s == ":=")
        {
            
            if (!isPfor()) return false;
            if (s == "else")
            {
                return isElse();
            }
            return true;
        }
        else
        {
            if (!isForFor()) return false;
            if (!isCfor()) return false;
            if (s == "else")
            {
                if (isElse()) return true;
                return false;
            }
            return true;
        }
    }
    
    { // разбор еще одного варианта СFor'a
        if (!isExpression()) return false;
        if (!isCfor()) return false;
        if (s == "else")
        {
            if (isElse()) return true;
            return false;
        }
        return true;
    }
}

bool isDoWhile()
{
    if (s == "do")
    {
        lexicalAnalyser(s, token);
        {
            if (isOperator())
            {
                if (s == "while")
                {
                    lexicalAnalyser(s, token);
                    if (s == "(")
                    {
                        lexicalAnalyser(s, token);
                        if (isExpression())
                        {
                            if (s == ")")
                            {
                                lexicalAnalyser(s, token);
                                if (s == ";")
                                {
                                    lexicalAnalyser(s, token);
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool isWhile()
{
    if (s == "while")
    {
        lexicalAnalyser(s, token);
        if (s == "(")
        {
            lexicalAnalyser(s, token);
            if (isExpression())
            {
                lexicalAnalyser(s, token);
                if (isOperator()) return true;
            }
        }
    }
    
    return false;
}

bool isIf()
{
    if (s == "if")
    {
        lexicalAnalyser(s, token);
        if (s == "(")
        {
            lexicalAnalyser(s, token);
            if (isExpression())
            {
                lexicalAnalyser(s, token);
                if (isOperator())
                {
                    if (s == "else")
                        return (isElse());
                    else return true;
                }
            }
        }
    }
    return false;
}

bool isSpecOperator()
{
    if (s == "for")
    {
        return (isFor());
    }
    if (s == "do")
    {
        return (isDoWhile());
    }
    if (s == "while")
    {
        return (isWhile());
    }
    if (s == "if")
    {
        return (isIf());
    }
    return false;
}

bool isType()
{
    if (s == "int" || s == "bool" || s == "double")
    {
        lexicalAnalyser(s, token);
        return true;
    }
    return false;
}

bool isSection()
{
    if (!isName()) return false;
    if (s == "=")
    {
        lexicalAnalyser(s, token);
        return isExpression();
    }
    return true;
}

bool isDescription()
{
    if (isType())
    {
        if (s != "main")
        {
            if (isSection())
            {
                while (s == ",")
                {
                    lexicalAnalyser(s, token);
                    if (!isSection()) return false;
                }
                bool fl;
                fl = (s == ";");
                lexicalAnalyser(s, token);
                return fl;
            }
        }
    }
    return false;
}

bool isDescription1()
{
    if (isSection())
    {
        while (s == ",")
        {
            lexicalAnalyser(s, token);
            if (!isSection()) return false;
        }
        bool fl;
        fl = (s == ";");
        lexicalAnalyser(s, token);
        return fl;
    }
    return false;
}

bool isCompOperator()
{
    if (s == "{")
    {
        lexicalAnalyser(s, token);
        if (!isOperator()) return false;
        while (s != "}")
        {
            if (in.eof()) return false; // ошибка несоответсвия количества скобок
            if (!isOperator()) return false;
        }
        if (s == "}")
        {
            lexicalAnalyser(s, token);
            return true;
        }
    }
    return false;
}

bool isElement()
{
    if (s == "<<")
    {
        lexicalAnalyser(s, token);
        if (token == "string")
        {
            lexicalAnalyser(s, token);
            return true;
        }
        if (s == "endl")
        {
            lexicalAnalyser(s, token);
            return true;
        }
        if (isExpression()) return true;
        return false;
    }
    if (s == ">>")
    {
        lexicalAnalyser(s, token);
        return isName();
    }
    return false;
}

bool isListOfElements()
{
    if (isElement())
    {
        bool fl = true;
        while (fl)
        {
            fl = isElement();
        }
        return true;
    }
    return false;
}

bool isCinout()
{
    if (s == "cinout")
    {
        lexicalAnalyser(s, token);
        if (!isListOfElements()) return false;
        if (s == ";")
        {
            lexicalAnalyser(s, token);
            return true;
        }
        return false;
    }
    return false;
}

bool isOperatorOfExpression()
{
    if (isExpression())
    {
        if (s == ";")
        {
            lexicalAnalyser(s, token);
            return true;
        }
        return false;
    }
    return false;
}

bool isOperator()
{
    if (s == "cinout") return isCinout();
    if (s == "do" || s == "for" || s == "while" || s == "if") return isSpecOperator();
    if (s == "int" || s == "bool" || s == "double") return isDescription();
    if (s == "{") return isCompOperator();
    return isOperatorOfExpression();
}

bool syntaxAnalyser()
{
    if (s == "int" || s == "bool" || s == "double")
    {
        if (s == "int")
        {
            lexicalAnalyser(s, token);
            if (s == "main")
            {
                lexicalAnalyser(s, token);
                if (s == "(")
                {
                    lexicalAnalyser(s, token);
                    if (s == ")")
                    {
                        lexicalAnalyser(s, token);
                        return isOperator();
                    }
                }
            }
            else
            {
                fl = true;
                if (!isDescription1()) return false;
                
                while (fl)
                {
                    if (!isDescription()) fl = false;
                }
                if (s == "main")
                {
                    lexicalAnalyser(s, token);
                    if (s == "(")
                    {
                        lexicalAnalyser(s, token);
                        if (s == ")")
                        {
                            lexicalAnalyser(s, token);
                            return isOperator();
                        }
                    }
                }
            }
        }
        fl = true;
        while (fl)
        {
            if (!isDescription()) fl = false;
        }
        if (s == "main")
        {
            lexicalAnalyser(s, token);
            if (s == "(")
            {
                lexicalAnalyser(s, token);
                if (s == ")")
                {
                    lexicalAnalyser(s, token);
                    return isOperator();
                }
            }
        }
    }
    return false;
}

//СЕМАНТИЧЕСКИЙ И ПОСТРОЕНИЕ ПОЛИЗА

struct cell
{
    string id = "";
    string type = "";
    long cint = 0;
    double cdouble = 0;
};

vector <cell> Tid; //Table of id
stack <string> S_value;
stack <opcell> S_op;

void checkBool()
{
    if (S_value.top() != "bool")
        error("ERROR in checkBool: значение выражения должно быть bool ",lex[lexc].line);
}

bool checkId (string id)
{
    for (int i = 0; i < Tid.size(); i++)
        if (Tid[i].id == id)
            return true;
    
    return false;
}

void addId (string id, string type)
{
    //если уже объявлена, то ошибка
    if (checkId(id))
    {
        string s_error = "ERROR in addId: переменная " + id + " уже объявлена";
        error(s_error,lex[lexc].line);
    }
    
    //добавляем
    cell newID;
    newID.id = id;
    newID.type = type;
    
    Tid.push_back(newID);
}

string getType (string id)
{
    for (int i = 0; i < Tid.size(); i++)
        if (Tid[i].id == id)
            return Tid[i].type;
    
    return "";
}

void checkOperation(opcell operation)
// проверяем на соответсвие и кидаем результат операции в стек
//если все норм, кидает в стек, иначе завершаем анализ из-за ошибки
{
    string result;
    string op = operation.op;
    long err_line = operation.line;
    
    if (S_value.empty())
    {
        string s_error = "ERROR in checkOperation: нет первого операнда для проверки операции " + op;
        error(s_error,lex[lexc].line);
    }
    
    string type1 = S_value.top();
    S_value.pop();
    
    if (op == "!")
    {
        if (type1 == "bool")
            result = "bool";
        else error("ERROR in checkOperation: тип операнда для ! должен быть bool",err_line);
        S_value.push(result);
        return;
    }
    
    if (op == "_")
    {
        if (type1 == "bool")
            error("ERROR in checkOperation: тип операнда для унарного минуса не может быть bool",err_line);
        result = type1;
        S_value.push(result);
        return;
    }
    
    if (op == "++" || op == "--")
    {
        if (type1 == "int")
            result = "int";
        else error("ERROR in checkOperation: тип операнда для инкремента/декремента не может быть bool",err_line);
        S_value.push(result);
        return;
    }
    
    //вытащить из стека второй операнд
    if (S_value.empty())
    {
        string s_error = "ERROR in checkOperation: нет второго операнда для проверки операции " + op;
        error(s_error,err_line);
    }
    string type2 = S_value.top();
    S_value.pop();
    
    
    if (op == "<" || op == ">" || op == ">=" || op == "<=" || op == "==" || op == "!=")
    {
        if (type1 == type2 || ( type1 != "bool" && type2 != "bool" ))
            result = "bool";
        else error("ERROR in checkOperation: значения операндов для сравнения не соответствуют друг другу",err_line);
        S_value.push(result);
        return;
    }
    
    
    if (op == "=") //только в double можем присвоить int, не наоборот
    {
        if (type1 == type2 || (type2 == "double" && type1 == "int" ) )
            result = type1;
        else error("ERROR in checkOperation: типы переменной и присваемового ей выражения не соответствуют друг другу",err_line);
        S_value.push(result);
        return;
    }
    
    if (op == ":=") //результат присваивания в pfor не нужен в стеке
    {
        if (!(type1 == "int" && type2 == "int"))
            error("ERROR in checkOperation: типы переменной и присваемового ей выражения отличны от int",err_line);
        return;
    }
    
    //рассматриваем в том числе сложение дабла и инта
    if (op == "-" || op == "+")
    {
        if ( type1 != "bool" && type2 != "bool")
            if (type1 == "int" && type2 == "int")
                result = "int";
            else result = "double";
            else error("ERROR in checkOperation: типы операндов для + или - не могут иметь тип bool",err_line);
        S_value.push(result);
        return;
    }
    
    if (op == "||" || op == "&&")
    {
        if (type1 == "bool" && type2 == "bool")
            result = "bool";
        else error("ERROR in checkOperation: операнды для логического И/ИЛИ должны иметь тип bool",err_line);
        S_value.push(result);
        return;
    }
    
    if (op == "*" || op == "/")
    {
        if (type1 != "bool" && type2 != "bool")
        {
            if (op == "*" && type1 == "int" && type2 == "int")
                result = "int";
            else result = "double";
        }
        else error("ERROR in checkOperation: типы операндов для * или / не могут быть bool",err_line);
        S_value.push(result);
        return;
    }
    
    if (op == "div" || op == "%")
    {
        if (type1 == "int" && type2 == "int")
            result = "int";
        else error("ERROR in checkOperation: операнды для div или mod(%) должны иметь тип int",err_line);
        S_value.push(result);
        return;
    }
    
    if (op == "^")
    {
        if (type1 != "bool" && type2 != "bool")
        {
            if (type1 == "int" && type2 == "int")
                result = "int";
            else result = "double";
        }
        else error("ERROR in checkOperation: типы операндов возведения в степень не могут быть bool",err_line);
        S_value.push(result);
        return;
    }
    
}

long priority(opcell operation)
{
    string op = operation.op;
    
    if (op == "++" || op == "--")
        return 9;
    if (op == "^")
        return 8;
    if (op == "!" || op == "_") //  "_" - это унарный минус
        return 7;
    if (op == "*" || op == "/" || op == "div" || op == "%")
        return 6;
    if (op == "+" || op == "-")
        return 5;
    if (op == "<" || op == ">" || op == "<=" || op == ">=")
        return 4;
    if (op == "==" || op == "!=")
        return 3;
    if (op == "&&")
        return 2;
    if (op == "||")
        return 1;
    if (op == "(")
        return 0;
    if (op == "=")
        return -1;
    
    string s_error = "ERROR in priority: неожиданная операция " + op;
    error(s_error,operation.line);
    
    return 0;
}

bool leftAssoc(string op)
{
    if (op == "^" || op == "++" || op == "--")
        return false;
    
    return true;
}

void removeId (long num)
//удаление последних num лобавленных переменных
{
    for (int i = 0; i < num; i++)
        Tid.erase(Tid.begin() + Tid.size()-1 );
}

// ДОБАВЛЕНИЕ ЯЧЕЕК

void addAdress (long num)
{
    pcell p;
    p.type = 0;
    p.str = "";
    p.cint = num;
    
    polis.push_back(p);
}

void addFalseAdress(long num)
{
    pcell p;
    p.type = 0;
    p.str = "#";
    p.cint = num;
    polis.push_back(p);
}

void addVal(string name)
{
    pcell p;
    p.type = 6;
    p.str = name;
    
    polis.push_back(p);
}

void addDeclaration(string type)
{
    pcell p;
    p.type = 1;
    p.str = type;
    
    polis.push_back(p);
}

void addString (string name)
{
    pcell p;
    p.str = name;
    p.type = 5;
    
    polis.push_back(p);
}

void addCinout(string cinout)
{
    pcell p;
    p.str = cinout;
    p.type = 1;
    
    polis.push_back(p);
}

void addRemove(long num)
{
    pcell p;
    p.type = 1;
    p.str = "remove";
    p.cint = num;
    
    removeId(num);
    polis.push_back(p);
}

void addOperation(string op, vector <pcell> & tpolis, long line) //встроен check_operation из семантического
{
    //проверка на то, не является ли минус/плюс унарным
    string s = lex[lexc-1].str;
    
    opcell operation;
    operation.op = op;
    operation.line = line;
    
    if (s == "<<" || s == "=" || s == "(" || s == "<=" || s == ">=" || s == "==" || s == "!=" || s == "<" || s == ">" || s == "+" || s == "-")
        unmin = true;
    else unmin = false;
    
    if (unmin)
    {
        if (op == "+")
            return;
        if (op == "-")
            op = "_";
    }
    
    
    if (op == "(")
    {
        S_op.push(operation);
        return;
    }
    if (op == ")")
    {
        pcell p;
        p.type = 1;
        p.str = S_op.top().op;
        opcell op;
        op = S_op.top();
        S_op.pop();
        
        while (p.str != "(")
        {
            checkOperation(op); //SEMANTIC
            tpolis.push_back(p);
            p.str = S_op.top().op;
            op = S_op.top();
            S_op.pop();
        }
        return;
    }
    
    if (S_op.empty())
    {
        S_op.push(operation);
        return;
    }
    
    long n = priority(operation);
    opcell operation2 = S_op.top();
    
    if (leftAssoc(operation.op))
        while (priority(operation2) >= n) //левоассоциативные
        {
            pcell p;
            p.type = 1;
            p.str = operation2.op;
            p.line = operation2.line;
            checkOperation(operation2); //SEMANTIC
            tpolis.push_back(p);
            S_op.pop();
            if (S_op.empty())
                break;
            operation2 = S_op.top();
        }
    else
        while (priority(operation2) > n) //правоассоциативные (степень и инкремент)
        {
            pcell p;
            p.type = 1;
            p.str = operation2.op;
            p.line = operation2.line;
            checkOperation(operation2); //SEMANTIC
            tpolis.push_back(p);
            S_op.pop();
            if (S_op.empty())
                break;
            operation2 = S_op.top();
        }
    
    S_op.push(operation);
    
}

// ПОСТРОЕНИЕ ПОЛИЗА

void buildExpression(vector <pcell> & tpolis)
{
    long open_sk = 0, close_sk = 0;
    while (lex[lexc].type == 6 || (lex[lexc].type >= 1 && lex[lexc].type <= 4)) //у ненужного другой номер типа (запятые, служ слова, вся хрень)
    {
        if (lex[lexc].str == "(")
            open_sk++;
        if (lex[lexc].str == ")")
            close_sk++;
        
        if (close_sk > open_sk)
            break;
        
        if (lex[lexc].type == 6) //кинуть тип в стек + проверить переменную + кинуть ее в полиз
        {
            if (!checkId(lex[lexc].str))
            {
                string s_error = "EROOR in buildExpression: переменная " + lex[lexc].str + " не описана";
                error(s_error,lex[lexc].line);
            }
            
            S_value.push(getType(lex[lexc].str));
            tpolis.push_back(lex[lexc]);
        }
        if (lex[lexc].type > 1 && lex[lexc].type < 5)
        {
            if (lex[lexc].type == 2)
                S_value.push("bool");
            if (lex[lexc].type == 3)
                S_value.push("int");
            if (lex[lexc].type == 4)
                S_value.push("double");
            tpolis.push_back(lex[lexc]);
        }
        if (lex[lexc].type == 1)
            addOperation(lex[lexc].str,tpolis,lex[lexc].line);
        lexc++;
    }
    while (!S_op.empty())
    {
        pcell p;
        p.type = 1;
        p.str = S_op.top().op;
        p.line = S_op.top().line;
        opcell operation = S_op.top();
        S_op.pop();
        checkOperation(operation);
        tpolis.push_back(p);
    }
}

void buildDeclaration(long & declared) //ставит lexc на начало след лексему после ;
//добавление описания в полиз + добавление в тид + проверка на то, не описана ли перем
{
    string type = lex[lexc].str;
    while (lex[lexc].str != ";")
    {
        lexc++;
        string name = lex[lexc++].str;
        
        if (checkId(name))
        {
            string s_error = "EROOR in buildDeclaration: переменная " + name + " уже описана";
            error(s_error,lex[lexc-1].line);
        }
        
        addVal(name);
        declared++;
        addDeclaration(type);
        addId(name,type);
        
        if ("=" == lex[lexc].str)
        {
            opcell operation;
            operation.op = "=";
            operation.line = lex[lexc].line;
            lexc++;
            addVal(name);
            S_value.push(type);
            buildExpression(polis);
            
            checkOperation(operation);
            pcell p;
            p.type = 1;
            p.str = "=";
            polis.push_back(p);
        }
    }
    lexc++;
}

void buildSimpOp();
void buildCompOp();

void buildWhile()
{
    lexc++;
    
    long expression_cell = polis.size();
    buildExpression(polis);
    checkBool();
    
    addFalseAdress(0);
    long adress_cell = polis.size() - 1;
    
    if (lex[lexc].str == "{")
        buildCompOp();
    else buildSimpOp();
    
    addAdress(expression_cell);
    polis[adress_cell].cint = polis.size();
}

void buildDoWhile()
{
    lexc++;
    
    long do_cell = polis.size();
    
    if (lex[lexc].str == "{")
        buildCompOp();
    else buildSimpOp();
    lexc += 2;
    
    buildExpression(polis);
    checkBool();
    
    pcell p;
    p.type = 1;
    p.str = "!";
    polis.push_back(p);
    
    addFalseAdress(do_cell);
}

void buildIf()
{
    lexc++;
    
    buildExpression(polis);
    checkBool();
    
    long else_cell = polis.size();
    addFalseAdress(0);
    
    if (lex[lexc].str == "{")
        buildCompOp();
    else
        buildSimpOp();
    
    if (lex[lexc].str == "else")
    {
        long not_else_cell = polis.size();
        addAdress(0);
        
        lexc++;
        
        polis[else_cell].cint = polis.size();
        
        if (lex[lexc].str == "{")
            buildCompOp();
        else
            buildSimpOp();
        
        polis[not_else_cell].cint = polis.size();
    }
    else
    {
        polis[else_cell].cint = polis.size();
    }
}

void buildCinout() //ставит lexc на начало след лексему после ;
{
    lexc++;
    while (lex[lexc].str != ";")
    {
        if (lex[lexc].str == ">>")
        {
            addVal(lex[++lexc].str);
            
            addCinout("in");
            lexc++;
        }
        else
        {
            lexc++;
            
            if (lex[lexc].type == 5)
                addString(lex[lexc++].str);
            else
                buildExpression(polis);
            
            addCinout("out");
        }
    }
    lexc++;
}

void buildCfor()
{
    //(B1;B2;B3) <body> = B1; while (B2) <body+B3>;
    //(B1;B2;B3) <body> else <elsebody>; = B1; if (B2) do <body+B3> while (B2); else <elsebody>;
    
    long declared = 0;
    if (lex[lexc].str == "bool" || lex[lexc].str == "int" || lex[lexc].str == "double")
        buildDeclaration(declared);
    else
    {
        buildExpression(polis);
        lexc++;
    }
    
    long cond_begin = polis.size();
    buildExpression(polis);
    checkBool();
    long cond_end = polis.size() - 1;
    
    lexc++;
    
    addFalseAdress(0);
    
    vector <pcell> expr3;
    buildExpression(expr3);
    
    lexc++;
    
    if (lex[lexc].str == "{")
        buildCompOp();
    else buildSimpOp();
    
    for (int i = 0; i < expr3.size(); i++)
        polis.push_back(expr3[i]);
    
    
    if (lex[lexc].str == "else")
    {
        for (long i = cond_begin; i <= cond_end; i++)
            polis.push_back(polis[i]);
        
        long body = polis.size();
        addFalseAdress(0);
        
        pcell p;
        p.str = "!";
        p.type = 1;
        polis.push_back(p);
        
        addFalseAdress(cond_end + 2);
        polis[cond_end + 1].cint = polis.size();
        lexc++;
        
        if (lex[lexc].str == "{")
            buildCompOp();
        else buildSimpOp();
        
        polis[body].cint = polis.size();
    }
    else
    {
        addAdress(cond_begin);
        
        polis[cond_end + 1].cint = polis.size();
    }
    
    if (declared > 0)
        addRemove(declared);
}

void buildPfor() //указатель на первой лексеме (имя)
{
    string counter = lex[lexc].str; //счетчик
    
    if (!checkId(counter))
        error("ERROR in buildPfor: счетчик для pfor не описан ранее",lex[lexc].line);
    
    if (getType(counter) != "int")
        error("ERROR in buildPfor: счетчик для pfor не типа int",lex[lexc].line);
    /*
     if (get_type(counter) == 'double')
     error("ERROR in buildPfor: счетчик для pfor не перечисляемого типа");
     */
    
    addVal(counter);
    lexc += 2;
    
    buildExpression(polis);
    
    pcell p;
    p.type = 1;
    p.str = "=";
    polis.push_back(p); //присвоить
    
    long cond_begin = polis.size();
    long cond_end;
    
    addVal(counter);
    bool dec = (lex[lexc++].str == "downto");
    
    buildExpression(polis);
    
    if (S_value.top() != "int")
        error("ERROR in buildPfor: после to/downto должно быть выражение типа int",lex[lexc].line);
    
    if (dec)
        p.str = ">=";
    else
        p.str = "<=";
    
    polis.push_back(p);
    cond_end = polis.size() - 1;
    
    addFalseAdress(0); //polis[cond_end + 1]
    
    if (lex[++lexc].str == "{")
        buildCompOp();
    else
        buildSimpOp();
    
    addVal(counter);
    
    if (dec) p.str = "--";
    else p.str = "++";
    
    polis.push_back(p);
    
    if (lex[lexc].str == "else")
    {
        for (long i = cond_begin; i <= cond_end; i++)
            polis.push_back(polis[i]);
        
        addFalseAdress(0);
        
        long not_else = polis.size() - 1;
        
        p.str = "!";
        polis.push_back(p);
        
        addFalseAdress(cond_end + 2);
        polis[cond_end + 1].cint = polis.size();
        
        lexc++;
        
        if (lex[lexc].str == "{")
            buildCompOp();
        else
            buildSimpOp();
        
        polis[not_else].cint = polis.size();
        
    }
    else
    {
        addAdress(cond_begin);
        
        polis[cond_end+1].cint = polis.size();
    }
}

void buildFor()
{
    lexc += 2;
    
    if (lex[lexc+1].str == ":=")
        buildPfor();
    else
        buildCfor();
}

void buildSimpOp()
{
    if (lex[lexc].str == "for")
    {
        buildFor();
        return;
    }
    if (lex[lexc].str == "while")
    {
        buildWhile();
        return;
    }
    if (lex[lexc].str == "do")
    {
        buildDoWhile();
        return;
    }
    if (lex[lexc].str == "cinout")
    {
        buildCinout();
        return;
    }
    if (lex[lexc].str == "if")
    {
        buildIf();
        return;
    }
    buildExpression(polis);
    lexc++;
}

void buildCompOp() //устанавливает на след. лексему после }
{
    lexc++;
    
    long declared = 0;
    
    while (lex[lexc].str != "}")
    {
        if (lex[lexc].str == "int" || lex[lexc].str == "double" || lex[lexc].str == "bool")
            buildDeclaration(declared);
        else
            buildSimpOp();
    }
    
    if (declared > 0)
        addRemove(declared);
    
    lexc++;
}

void buildMain()
{
    long declared = 0;
    
    while ( (lex[lexc].str == "int" && lex[lexc+1].str != "main") || lex[lexc].str == "bool" || lex[lexc].str == "double")
        buildDeclaration(declared);
    
    lexc += 4;
    buildCompOp();
    
    if (declared > 0)
        addRemove(declared);
}

void printPolis()
{
    for (int i = 0; i < polis.size(); i++)
    {
        cout << i << "    ";
        if (polis[i].type == 6)
            cout << polis[i].str << endl;
        
        if (polis[i].type == 0)
        {
            if (polis[i].str == "#")
                cout << "if false ";
            cout << "go " << polis[i].cint << endl;
        }
        
        if (polis[i].type == 3)
            cout << polis[i].cint << endl;
        
        if (polis[i].cdouble == 2)
            cout << (polis[i].cint == 1) << endl;
        
        if (polis[i].type == 4)
            cout << polis[i].cdouble << endl;
        
        if (polis[i].type == 1)
        {
            cout << polis[i].str;
            if (polis[i].str == "remove") cout << " " << polis[i].cint;
            cout << endl;
        }
        
        if (polis[i].type == 5)
        {
            if (polis[i].str == "\n")
                cout << "string endl \n";
            else cout << "string " << polis[i].str << endl;
        }
        
        if (polis[i].type > 6)
            cout << polis[i].str << "ERROR in POLIS: polis[i].type > 6 !!! ";
    }
}

stack <pcell> S_temp;

// ЗАПУСК ПОЛИЗА

int findInTID (string name,long line)
{
    int i;
    for (i = 0; i < Tid.size(); i++)
        if (Tid[i].id == name)
            break;
    if (i >= Tid.size())
    {
        string s_error = "ERROR in findInTID: переменная " + name + "не объявлена";
        error(s_error,line);
    }
    
    return i;
}

void replaceWithValue (pcell & p0) //заменяет ячейку с переменной ее значениями
//найти имя в ТИД - запомнить номер - скопировать тип - скопировать значение
{
    int i = findInTID(p0.str,p0.line);
    
    p0.cdouble = Tid[i].cdouble;
    p0.cint = Tid[i].cint;
    p0.str = "";
    
    if (Tid[i].type == "bool")
        p0.type = 2;
    if (Tid[i].type == "int")
        p0.type = 3;
    if (Tid[i].type == "double")
        p0.type = 4;
}

void runOperation()
{
    string op = polis[k].str;
    
    if (op == "remove")
    {
        removeId(polis[k].cint);
        return;
    }
    
    pcell result;
    
    if (S_temp.empty())
        error("ERROR in runOperation: S_temp пуст",polis[k].line);
    
    pcell r_op = S_temp.top();
    S_temp.pop();
    
    if (op == "out")
    {
        if (r_op.type < 2) error("ERROR in RunOperation: тип операнда для cout меньше 2",r_op.line);
        
        if (r_op.type == 6)
            replaceWithValue(r_op);
        if (r_op.type == 5)
            cout << r_op.str;
        if (r_op.type == 4)
            cout << r_op.cdouble;
        if (r_op.type == 3)
            cout << r_op.cint;
        if (r_op.type == 2)
            cout << (r_op.cint == 1);
        /*
         if (r_op.type == 2)
         {
         if (r_op.cint == 1)
         cout << "true";
         else cout << "false";
         }
         */
        return;
    }
    
    if (op == "in")
    {
        if (r_op.type != 6) error("ERROR in RunOperation: операнд для cin не является переменной",r_op.line);
        
        int i = findInTID(r_op.str,polis[k].line);
        if (Tid[i].type == "double")
            cin >> Tid[i].cdouble;
        if (Tid[i].type == "int")
        {
            cin >> Tid[i].cint;
            char c;
            cin >> c;
            if (c == '.')
                error("ERROR: нельзя присвоить в переменную типа int double",0);
        }
        if (Tid[i].type == "bool")
        {
            bool fl;
            cin >> fl;
            if (fl) Tid[i].cint = 1;
            else Tid[i].cint = 0;
        }
        
        return;
    }
    
    if (op == "_")
    {
        if (r_op.type == 6)
            replaceWithValue(r_op);
        
        result.type = r_op.type;
        result.cint = - r_op.cint;
        result.cdouble = - r_op.cdouble;
        
        S_temp.push(result);
        return;
    }
    
    //r_op - имя переменной //op -  название типа значение для создаваемой переменной
    if (op == "int" || op == "double" || op == "bool")
    {
        cell ptid;
        ptid.type = op;
        ptid.id = r_op.str;
        
        Tid.push_back(ptid);
        return;
    }
    
    if (op == "!")
    {
        if (r_op.type == 6)
            replaceWithValue(r_op);
        
        result.type = 2;
        if (r_op.cint == 1)
            result.cint = 0;
        else result.cint = 1;
        
        S_temp.push(result);
        return;
    }
    
    if (op == "++" || op == "--")
    {
        result.type = 3;
        if (r_op.type == 6)
        {
            int i = findInTID(r_op.str,polis[k].line);
            
            if (op == "++")
                result.cint = ++Tid[i].cint;
            else result.cint = --Tid[i].cint;
            S_temp.push(result);
        }
        else
        {
            if (op == "++")
                result.cint = r_op.cint + 1;
            else result.cint = r_op.cint - 1;
            S_temp.push(result);
        }
        return;
    }
    
    //вытащить из стека второй операнд
    if (S_temp.empty())
        error("ERROR in RunOperation: S_temp пуст",polis[k].line);
    
    pcell l_op = S_temp.top();
    S_temp.pop();
    if (r_op.type == 6)
        replaceWithValue(r_op); //нам потребуется только значение
    
    if (op == "<" || op == ">" || op == ">=" || op == "<=" || op == "==" || op == "!=")
    {
        result.type = 2;
        if (l_op.type == 6)
            replaceWithValue(l_op);
        
        double a,b;
        bool res = false;
        if (r_op.type == 4)
            a = r_op.cdouble;
        else a = r_op.cint;
        
        if (l_op.type == 4)
            b = l_op.cdouble;
        else b = l_op.cint;
        
        if (op == "<")
            res = b < a;
        if (op == "<=")
            res = b <= a;
        if (op == ">")
            res = b > a;
        if (op == ">=")
            res = b >= a;
        if (op == "==")
            res = b == a;
        if (op == "!=")
            res = b != a;
        
        if (res) result.cint = 1;
        else result.cint = 0;
        S_temp.push(result);
        return;
        
    }
    
    
    if (op == "=" || op == ":=") //только в double можем присвоить int, не наоборот
    {
        if (l_op.type != 6) error("ERROR in RunOperation: можно присваивать только в переменную",l_op.line);
        long i = findInTID(l_op.str,polis[k].line);
        
        Tid[i].cint = r_op.cint;
        Tid[i].cdouble = r_op.cdouble;
        
        result = r_op; //кидает в стек то, что присвоили = результат присваивания
        if (op == "=")
            S_temp.push(result); //для := нет смысла кидать результат в стек
        return;
    }
    
    
    //рассматриваем в том числе сложение дабла и инта
    if (op == "-" || op == "+" || op == "*")
    {
        if (l_op.type == 6)
            replaceWithValue(l_op);
        
        if (r_op.type == 3 && l_op.type == 3)
        {
            result.type = 3;
            long a = l_op.cint, b = r_op.cint;
            if (op == "+")
                result.cint = a + b;
            if (op == "-")
                result.cint = a - b;
            if (op == "*")
                result.cint = a * b;
        }
        else
        {
            result.type = 4;
            double a,b;
            if (l_op.type == 3)
                a = l_op.cint;
            else a = l_op.cdouble;
            
            if (r_op.type == 3)
                b = r_op.cint;
            else b = r_op.cdouble;
            
            if (op == "+")
                result.cdouble = a + b;
            if (op == "-")
                result.cdouble = a - b;
            if (op == "*")
                result.cdouble = a * b;
        }
        
        S_temp.push(result);
        return;
    }
    
    if (op == "||" || op == "&&")
    {
        if (l_op.type == 6)
            replaceWithValue(l_op);
        
        bool a = (l_op.cint == 1), b = (r_op.cint == 1), res = false;
        
        if (op == "||")
            res = a || b;
        if (op == "&&")
            res = a && b;
        
        result.type = 2;
        if (res) result.cint = 1;
        else result.cint = 0;
        
        S_temp.push(result);
        return;
    }
    
    if (op == "/") //всегда double результат
    {
        if (l_op.type == 6)
            replaceWithValue(l_op);
        
        result.type = 4;
        double a,b;
        
        if (l_op.type == 3)
            a = l_op.cint;
        else a = l_op.cdouble;
        
        if (r_op.type == 3)
            b = r_op.cint;
        else b = r_op.cdouble;
        
        if (b == 0)
            error("ERROR in RunOperation: деление на ноль",r_op.line);
        
        result.cdouble = (double) a/b;
        
        S_temp.push(result);
        return;
    }
    
    if (op == "div" || op == "%")
    {
        if (l_op.type == 6)
            replaceWithValue(l_op);
        
        result.type = 3;
        long a = l_op.cint, b = r_op.cint;
        
        if (b == 0)
            error("ERROR in RunOperation: деление на ноль",r_op.line);
        
        if (op == "div")
            result.cint = a/b;
        if (op == "%")
            result.cint = a % b;
        
        S_temp.push(result);
        return;
    }
    
    if (op == "^")
    {
        if (l_op.type == 6)
            replaceWithValue(l_op);
        
        double a,b;
        if (l_op.type == 3)
            a = l_op.cint;
        else a = l_op.cdouble;
        
        if (r_op.type == 3)
            b = r_op.cint;
        else b = r_op.cdouble;
        
        if (r_op.type == 3 && l_op.type == 3)
        {
            result.type = 3;
            result.cint = pow(a,b);
        }
        else
        {
            result.type = 4;
            result.cdouble = pow(a,b);
        }
        
        S_temp.push(result);
        return;
    }
    
    //добавить операции ввода, вывода, объявления (добавить в тид), удаления из тида и прочее
}

void runCell(long & k)
{
    if (polis[k].type == 0)
    {
        if (polis[k].str == "#")
        {
            if (S_temp.top().cint == 0)
                k = polis[k].cint;
            else k++;
            
        }
        else k = polis[k].cint;
        return;
    }
    if (polis[k].type >= 2 && polis[k].type <= 6)
    {
        S_temp.push(polis[k]);
        k++;
        return;
    }
    if (polis[k].type == 1)
    {
        runOperation();
        k++;
        return;
    }
    cout << "ERR тип ячейки не в диапазоне от 0 до 6" << endl;
}

void runPolis()
{
    while (k < polis.size())
        runCell(k);
}

int main()
{
    in.open("Fin.txt");
    if (in.eof())
        error("пустой файл",0);
    
    in.get(c);
    lexicalAnalyser(s, token);
    if (!syntaxAnalyser())
        error("ERROR in syntaxAnalyser",line);
    if (!in.eof())
    {
        if (boolAnalyser(s, token))
            error("ERROR: встречен неожиданный символ после основной функции main",line);
    }
    buildMain();
    //cout << "УСПЕХ" << endl;
    printPolis();
    
    runPolis();
}
