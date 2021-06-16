#pragma once
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "context.h"

namespace step1 {

class Node {
  public:
    virtual ~Node();
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    void printIndentation(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out) = 0;
};

class NExpression : public Node {
  public:
    virtual void ir(Context &ctx, std::ostream &out);
    virtual int eval(Context &ctx);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual std::string getcond(Context &ctx, std::ostream &out, std::string &l_true, std::string &l_false);
    virtual int detect(std::string);
};

class NStatement : public Node {
};

class NDeclare : public Node {
};

class NIdentifier : public NExpression {
  public:
    std::string name;
    NIdentifier(const std::string& name);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual int eval(Context &ctx);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual std::string left_getvar(Context &ctx, std::ostream &out);
    virtual int detect(std::string);
};

class NConditionExpression : public NExpression {
  public:
    NExpression& value;
    NConditionExpression(NExpression& value);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual std::string getcond(Context &ctx, std::ostream &out, std::string &l_true, std::string &l_false);
    virtual int detect(std::string);
};

class NBinaryExpression : public NExpression {
  public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryExpression(NExpression& lhs, int op, NExpression& rhs);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual int eval(Context &ctx);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual std::string getcond(Context &ctx, std::ostream &out, std::string &l_true, std::string &l_false);
    virtual int detect(std::string);
};

class NUnaryExpression : public NExpression {
  public:
    int op;
    NExpression& rhs;
    NUnaryExpression(int op, NExpression& rhs);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual int eval(Context &ctx);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual int detect(std::string);
};

class NFunctionCallArgList : public NExpression {
  public:
    std::vector<NExpression*> args;
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
};

class NFunctionCall : public NExpression {
  public:
    NIdentifier& name;
    NFunctionCallArgList& args;
    NFunctionCall(NIdentifier& name, NFunctionCallArgList& args);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual void ir(Context &ctx, std::ostream &out);
    void ir_param(Context &ctx, std::ostream &out);
    virtual int detect(std::string);
};

class NNumber : public NExpression {
  public:
    int value;
    NNumber(const std::string& value);
    NNumber(int value);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual int eval(Context &ctx);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual int detect(std::string);
};

class NBlock : public NStatement {
  public:
    std::vector<NStatement*> statements;
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NAssignment : public NStatement {
  public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NIfElseStatement : public NStatement {
  public:
    NConditionExpression& cond;
    NStatement& thenstmt;
    NStatement& elsestmt;
    NIfElseStatement(NConditionExpression& cond, NStatement& thenstmt, NStatement& elsestmt);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NWhileStatement : public NStatement {
  public:
    NConditionExpression& cond;
    NStatement& dostmt;
    NWhileStatement(NConditionExpression& cond, NStatement& dostmt);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NBreakStatement : public NStatement {
  public:
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NContinueStatement : public NStatement {
  public:
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NReturnStatement : public NStatement {
  public:
    NExpression* value;
    NReturnStatement(NExpression* value = NULL);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NEvalStatement : public NStatement {
  public:
    NExpression& value;
    NEvalStatement(NExpression& value);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NVoidStatement : public NStatement {
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NDeclareStatement : public NStatement {
  public:
    std::vector<NDeclare*> list;
    int type;
    NDeclareStatement(int type);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NArrayDeclareInitValue : public NExpression {
  public:
    bool is_number;
    NExpression* value;
    std::vector<NArrayDeclareInitValue*> value_list;
    NArrayDeclareInitValue(bool is_number, NExpression* value);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    void array_value(Context &ctx, std::ostream &out, std::string &name, const std::vector<int> &shape, int d, int offset, int is_const, std::vector<int> &const_value);
};

class NVarDeclareWithInit : public NDeclare {
  public:
    NIdentifier& name;
    NExpression& value;
    bool is_const;
    NVarDeclareWithInit(NIdentifier& name, NExpression& value, bool is_const = false);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NVarDeclare : public NDeclare {
  public:
    NIdentifier& name;
    NVarDeclare(NIdentifier& name);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NArrayIdentifier : public NIdentifier {
  public:
    NIdentifier& name;
    std::vector<NExpression*> shape;
    NArrayIdentifier(NIdentifier& name);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual int eval(Context &ctx);
    virtual std::string getvar(Context &ctx, std::ostream &out, std::string ph = "");
    virtual std::string left_getvar(Context &ctx, std::ostream &out);
    virtual int detect(std::string);
};

class NArrayDeclareWithInit : public NDeclare {
  public:
    NArrayIdentifier& name;
    NArrayDeclareInitValue& value;
    bool is_const;
    NArrayDeclareWithInit(NArrayIdentifier& name, NArrayDeclareInitValue& value, bool is_const = false);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NArrayDeclare : public NDeclare {
  public:
    NArrayIdentifier& name;
    NArrayDeclare(NArrayIdentifier& name);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NFunctionDefineArg : public NExpression {
  public:
    int type;
    NIdentifier& name;
    NFunctionDefineArg(int type, NIdentifier& name);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
};

class NFunctionDefineArgList : public NExpression {
  public:
    std::vector<NFunctionDefineArg*> list;
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
};

class NFunctionDefine : public Node {
  public:
    int return_type;
    NIdentifier& name;
    NFunctionDefineArgList& args;
    NBlock& body;
    NFunctionDefine(int return_type, NIdentifier& name, NFunctionDefineArgList& args, NBlock& body);
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

class NRoot : public Node {
  public:
    std::vector<Node*> body;
    virtual void print(int indentation = 0, bool end = false, std::ostream& out = std::cerr);
    virtual void ir(Context &ctx, std::ostream &out);
};

std::vector<int> eval_shape(Context&, std::vector<NExpression*>);

}
// namespace step1
