module;
#include "gen/GIScriptLexer.h"
#include "gen/GIScriptParser.h"
module GIScript;

import script;

using namespace Ugc::Script;

ASTNode::ASTNode()
{
}

ASTNode::~ASTNode()
{
}

DeclarationNode::DeclarationNode()
{
}

RootNode::RootNode(std::vector<std::unique_ptr<DeclarationNode>> declarations, std::vector<std::unique_ptr<FunctionNode>> global_functions) : declarations(std::move(declarations)), global_functions(std::move(global_functions))
{
}

StatementNode::StatementNode()
{
}

NopStatement::NopStatement()
{
}

BreakStatement::BreakStatement()
{
}

ExpressionNode::ExpressionNode()
{
}

BlockNode::BlockNode(std::vector<std::unique_ptr<StatementNode>> statements) : statements(std::move(statements))
{
}

Variable::Variable(const std::string& id, VarType type, std::unique_ptr<ExpressionNode> value) : id(id), type(std::move(type)), value(std::move(value))
{
}

EventNode::EventNode(const std::string& event, std::vector<Variable> parameters, BlockNode body) : event(event), parameters(std::move(parameters)), body(std::move(body))
{
	struct Checker : ASTVisitor
	{
		void VisitReturn(const std::any& value) override
		{
			throw std::runtime_error("Return must be the last statement in the function");
		}
	} checker;
	this->body.Visit(checker);
}

FunctionNode::FunctionNode(const std::string& name, std::optional<VarType> ret, std::vector<Variable> parameters, BlockNode body) : name(name), parameters(std::move(parameters)), ret(std::move(ret)), body(std::move(body))
{
	struct Checker : ASTVisitor
	{
		void VisitReturn(const std::any& value) override
		{
			throw std::runtime_error("Return must be the last statement in the function");
		}
	} checker;
	for (auto& s : this->body.statements)
	{
		if (typeid(*s) == typeid(Return))
		{
			if (s != this->body.statements.back()) throw std::runtime_error("Return must be the last statement in the function");
		}
		else s->Visit(checker);
	}
}

Return::Return(std::unique_ptr<ExpressionNode> expr) : expr(std::move(expr))
{
}

VarDef::VarDef(std::vector<Variable> vars) : vars(std::move(vars))
{
}

ExprStatement::ExprStatement(std::unique_ptr<ExpressionNode> expr) : expr(std::move(expr))
{
}

IfStatement::IfStatement(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<StatementNode> then, std::unique_ptr<StatementNode> otherwise) : condition(std::move(condition)), then(std::move(then)), otherwise(std::move(otherwise))
{
}

CaseNode::CaseNode(std::unique_ptr<ExpressionNode> literal, std::vector<std::unique_ptr<StatementNode>> statements) : literal(std::move(literal)), statements(std::move(statements))
{
}

SwitchStatement::SwitchStatement(std::unique_ptr<ExpressionNode> expr, std::vector<std::unique_ptr<CaseNode>> cases, std::unique_ptr<CaseNode> default_case) : expr(std::move(expr)), cases(std::move(cases)), default_case(std::move(default_case))
{
}

WhileStatement::WhileStatement(std::unique_ptr<ExpressionNode> expr, std::unique_ptr<StatementNode> body) : expr(std::move(expr)), body(std::move(body))
{
}

ForStatement::ForStatement(std::unique_ptr<StatementNode> init, std::unique_ptr<ExpressionNode> condition, std::unique_ptr<StatementNode> iteration, std::unique_ptr<StatementNode> body) : init(std::move(init)), condition(std::move(condition)), iteration(std::move(iteration)), body(std::move(body))
{
}

ForEachStatement::ForEachStatement(VarType type, const std::string& def, std::unique_ptr<ExpressionNode> iterable, std::unique_ptr<StatementNode> body) : type(std::move(type)), def(def), iterable(std::move(iterable)), body(std::move(body))
{
}

Literal::Literal(Type type, std::any value) : type(type), value(std::move(value))
{
}

Literal Literal::BitwiseNOT() const
{
	switch (type)
	{
	case Int:
		return Literal(Int, ~std::any_cast<int64_t>(value));
	default:
		throw std::runtime_error("Invalid unary operation");
	}
}

Literal Literal::LogicalNOT() const
{
	switch (type)
	{
	case Bool:
		return Literal(Bool, !std::any_cast<bool>(value));
	default:
		throw std::runtime_error("Invalid unary operation");
	}
}

Literal Literal::Negate() const
{
	switch (type)
	{
	case Int:
		return Literal(Int, -std::any_cast<int64_t>(value));
	case Float:
		return Literal(Float, -std::any_cast<float>(value));
	default:
		throw std::runtime_error("Invalid unary operation");
	}
}


Identifier::Identifier(const std::string& id) : id(id)
{
}

CallExpr::CallExpr(std::unique_ptr<ExpressionNode> expr, std::vector<std::unique_ptr<ExpressionNode>> args, std::optional<VarType> type) : expr(std::move(expr)), args(std::move(args)), type(std::move(type))
{
}

Increment::Increment(std::unique_ptr<ExpressionNode> expr, bool inv, bool pre) : expr(std::move(expr)), pre(pre), inv(inv)
{
}

MemberExpr::MemberExpr(std::unique_ptr<ExpressionNode> expr, std::unique_ptr<ExpressionNode> member, std::optional<VarType> type) : expr(std::move(expr)), member(std::move(member)), type(std::move(type))
{
}

Assignment::Assignment(std::unique_ptr<ExpressionNode> ref, std::unique_ptr<ExpressionNode> expr, Op op) : ref(std::move(ref)), expr(std::move(expr)), op(op)
{
}

UnaryExpr::UnaryExpr(Op op, std::unique_ptr<ExpressionNode> expr) : op(op), expr(std::move(expr))
{
}

BinaryExpr::BinaryExpr(Op op, std::unique_ptr<ExpressionNode> l, std::unique_ptr<ExpressionNode> r) : op(op), l(std::move(l)), r(std::move(r))
{
}

TernaryExpr::TernaryExpr(std::unique_ptr<ExpressionNode> e1, std::unique_ptr<ExpressionNode> e2, std::unique_ptr<ExpressionNode> e3) : e1(std::move(e1)), e2(std::move(e2)), e3(std::move(e3))
{
}

ChainExpr::ChainExpr(std::vector<std::unique_ptr<ExpressionNode>> elements) : elements(std::move(elements))
{
}

CastExpr::CastExpr(VarType type, std::unique_ptr<ExpressionNode> expr) : type(std::move(type)), expr(std::move(expr))
{
}

ConstructExpr::ConstructExpr(VarType type, std::vector<std::unique_ptr<ExpressionNode>> initializers) : type(std::move(type)), initializers(std::move(initializers))
{
}

InitializerList::InitializerList(std::vector<std::unique_ptr<ExpressionNode>> initializers) : initializers(std::move(initializers))
{
}

class FastFailListener :public antlr4::ANTLRErrorListener
{
public:
	void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol, size_t line, size_t charPositionInLine, const std::string& msg, std::exception_ptr e) override
	{
		throw std::runtime_error(std::format("Syntax error at line {}, char {}: {}", std::to_string(line), std::to_string(charPositionInLine), msg));
	}

	void reportAmbiguity(antlr4::Parser* recognizer, const antlr4::dfa::DFA& dfa, size_t startIndex, size_t stopIndex, bool exact, const antlrcpp::BitSet& ambigAlts, antlr4::atn::ATNConfigSet* configs) override
	{
	}

	void reportAttemptingFullContext(antlr4::Parser* recognizer, const antlr4::dfa::DFA& dfa, size_t startIndex, size_t stopIndex, const antlrcpp::BitSet& conflictingAlts, antlr4::atn::ATNConfigSet* configs) override
	{
	}

	void reportContextSensitivity(antlr4::Parser* recognizer, const antlr4::dfa::DFA& dfa, size_t startIndex, size_t stopIndex, size_t prediction, antlr4::atn::ATNConfigSet* configs) override
	{
	}
};

std::unique_ptr<ASTNode> Ugc::Script::Parse(const std::string& code)
{
	antlr4::ANTLRInputStream stream(code);
	GIScriptLexer lexer(&stream);
	lexer.removeErrorListeners();
	antlr4::CommonTokenStream tokens(&lexer);
	GIScriptParser parser(&tokens);
	parser.removeErrorListeners();
	FastFailListener l;
	lexer.addErrorListener(&l);
	parser.addErrorListener(&l);
	GI::Script::Parser p;
	p.visitProgram(parser.program());
	if (auto t = tokens.LT(1); t->getType() != antlr4::Token::EOF) throw std::runtime_error(std::format("Unexpected token '{}' at line {}:{}.", t->getText(), t->getLine(), t->getCharPositionInLine()));
	return p.Release();
}

bool VarType::operator==(const VarType& other) const
{
	if (type != other.type) return false;
	if (type == Guid && std::any_cast<GuidEx>(extra) != std::any_cast<GuidEx>(other.extra)) return false;
	if (type == List && std::any_cast<VarType>(extra) != std::any_cast<VarType>(other.extra)) return false;
	if (type == Map)
	{
		auto& [k, v] = std::any_cast<const MapEx&>(extra);
		auto& [k2, v2] = std::any_cast<const MapEx&>(other.extra);
		return k == k2 && v == v2;
	}
	return true;
}

void RootNode::Visit(ASTVisitor& visitor)
{
	for (auto& c : declarations) c->Visit(visitor);
}

void BlockNode::Visit(ASTVisitor& visitor)
{
	visitor.scope.enter();
	for (auto& s : statements) s->Visit(visitor);
	visitor.scope.exit();
}

void EventNode::Visit(ASTVisitor& visitor)
{
	visitor.scope.enter();
	visitor.VisitEvent(event, parameters);
	body.Visit(visitor);
	visitor.scope.exit();
}

void FunctionNode::Visit(ASTVisitor& visitor)
{
	visitor.scope.enter();
	visitor.VisitFunction(name, ret, parameters);
	body.Visit(visitor);
	visitor.scope.exit();
}

void Return::Visit(ASTVisitor& visitor)
{
	visitor.VisitReturn(expr ? expr->Eval(visitor) : std::any{});
}

void ExprStatement::Visit(ASTVisitor& visitor)
{
	visitor.VisitExprStatement(expr->Eval(visitor));
}

void IfStatement::Visit(ASTVisitor& visitor)
{
	visitor.scope.enter();
	auto value = condition->Eval(visitor);
	visitor.VisitIfStatement(Start, value);
	then->Visit(visitor);
	if (otherwise)
	{
		visitor.VisitIfStatement(Else, value);
		otherwise->Visit(visitor);
	}
	visitor.VisitIfStatement(End, value);
	visitor.scope.exit();
}

void CaseNode::Visit(ASTVisitor&)
{
	throw std::exception("Invalid call");
}

void SwitchStatement::Visit(ASTVisitor& visitor)
{
	auto value = expr->Eval(visitor);
	visitor.VisitSwitchStatement(cases.size(), value, false);
	auto process = [&](const CaseNode* c, bool is_default = false) mutable
		{
			visitor.scope.enter();
			visitor.VisitCase(is_default ? std::any{} : c->literal->Eval(visitor), value);
			for (auto& s : c->statements) s->Visit(visitor);
			visitor.scope.exit();
		};
	for (auto& c : cases) process(c.get());
	if (default_case) process(default_case.get(), true);
	visitor.VisitSwitchStatement(cases.size(), value, true);
}

void WhileStatement::Visit(ASTVisitor& visitor)
{
	visitor.scope.enter();
	auto value = expr->Eval(visitor);
	visitor.VisitWhile(value, false);
	body->Visit(visitor);
	visitor.VisitWhile(value, true);
	visitor.scope.exit();
}

void ForStatement::Visit(ASTVisitor& visitor)
{
	visitor.scope.enter();
	if (init) init->Visit(visitor);
	std::any value;
	if (condition) value = condition->Eval(visitor);
	visitor.VisitFor(value, false);
	body->Visit(visitor);
	if (iteration) iteration->Visit(visitor);
	visitor.VisitFor(value, true);
	visitor.scope.exit();
}

void ForEachStatement::Visit(ASTVisitor& visitor)
{
	visitor.scope.enter();
	auto value = iterable->Eval(visitor);
	visitor.VisitForEachStart(type, def, value);
	body->Visit(visitor);
	visitor.VisitForEachEnd(value);
	visitor.scope.exit();
}

void NopStatement::Visit(ASTVisitor& visitor)
{
}

void BreakStatement::Visit(ASTVisitor& visitor)
{
	visitor.VisitBreak();
}

void ExpressionNode::Visit(ASTVisitor&)
{
	throw std::exception("Invalid call");
}

void VarDef::Visit(ASTVisitor& visitor)
{
	for (auto& v : vars)
	{
		std::any value;
		if (v.Value()) value = v.Value()->Eval(visitor);
		auto type = v.Type();
		if (type.type == VarType::Unknown) type = visitor.TypeInference(value);
		visitor.VisitVarDef(v.Id(), std::move(type), value);
	}
}

std::any Literal::Eval(ASTVisitor& visitor)
{
	return visitor.VisitLiteral(type, value);
}

std::any Identifier::Eval(ASTVisitor& visitor)
{
	return visitor.VisitIdentifier(id);
}

std::any CallExpr::Eval(ASTVisitor& visitor)
{
	std::vector<std::any> args;
	for (auto& a : this->args) args.push_back(a->Eval(visitor));
	return visitor.VisitCall(expr->Eval(visitor), args, type);
}

std::any Increment::Eval(ASTVisitor& visitor)
{
	return visitor.VisitIncrement(expr->Eval(visitor), inv, pre);
}

std::any MemberExpr::Eval(ASTVisitor& visitor)
{
	return visitor.VisitMemberAccess(expr->Eval(visitor), member->Eval(visitor), type);
}

std::any Assignment::Eval(ASTVisitor& visitor)
{
	return visitor.VisitAssignment(ref->Eval(visitor), op, expr->Eval(visitor));
}

std::any UnaryExpr::Eval(ASTVisitor& visitor)
{
	return visitor.VisitUnary(op, expr->Eval(visitor));
}

std::any BinaryExpr::Eval(ASTVisitor& visitor)
{
	return visitor.VisitBinary(op, l->Eval(visitor), r->Eval(visitor));
}

std::any TernaryExpr::Eval(ASTVisitor& visitor)
{
	return visitor.VisitTernary(e1->Eval(visitor), e2->Eval(visitor), e3->Eval(visitor));
}

std::any ChainExpr::Eval(ASTVisitor& visitor)
{
	std::any result;
	for (auto& e : elements) result = e->Eval(visitor);
	return result;
}

std::any CastExpr::Eval(ASTVisitor& visitor)
{
	return visitor.VisitCast(type, expr->Eval(visitor));
}

std::any ConstructExpr::Eval(ASTVisitor& visitor)
{
	std::vector<std::any> args;
	for (auto& a : initializers) args.push_back(a->Eval(visitor));
	return visitor.VisitConstruct(type, args);
}

std::any InitializerList::Eval(ASTVisitor& visitor)
{
	std::vector<std::any> values;
	for (auto& a : initializers) values.push_back(a->Eval(visitor));
	return visitor.VisitInitializerList(values);
}
