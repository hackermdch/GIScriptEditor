module;
#include "gen/GIScriptBaseVisitor.h"
export module script;

import std;
import GIScript;

export namespace GI::Script
{
	class Parser : public GIScriptBaseVisitor
	{
		std::vector<std::unique_ptr<Ugc::Script::DeclarationNode>> declarations;
		std::vector<std::unique_ptr<Ugc::Script::FunctionNode>> global_functions;

		std::any visitEvent(GIScriptParser::EventContext* context) override;
		std::any visitFunction(GIScriptParser::FunctionContext* context) override;
		std::any visitBlock(GIScriptParser::BlockContext* context) override;
		std::any visitStatement(GIScriptParser::StatementContext* context) override;
		std::any visitVarDef(GIScriptParser::VarDefContext* context) override;
		std::any visitIf(GIScriptParser::IfContext* context) override;
		std::any visitSwitch(GIScriptParser::SwitchContext* context) override;
		std::any visitCase(GIScriptParser::CaseContext* context) override;
		std::any visitDefault(GIScriptParser::DefaultContext* context) override;
		std::any visitWhile(GIScriptParser::WhileContext* context) override;
		std::any visitFor(GIScriptParser::ForContext* context) override;
		std::any visitForEach(GIScriptParser::ForEachContext* context) override;
		std::any visitReturn(GIScriptParser::ReturnContext* context) override;

		std::any visitIntegerLiteral(GIScriptParser::IntegerLiteralContext* context) override;
		std::any visitFloatLiteral(GIScriptParser::FloatLiteralContext* context) override;
		std::any visitStringLiteral(GIScriptParser::StringLiteralContext* context) override;
		std::any visitIdentifierLiteral(GIScriptParser::IdentifierLiteralContext* context) override;
		std::any visitKeywordLiteral(GIScriptParser::KeywordLiteralContext* context) override;
		std::any visitTypeInitializer(GIScriptParser::TypeInitializerContext* context) override;

		std::any visitPostfix(GIScriptParser::PostfixContext* context) override;
		std::any visitFunctionCall(GIScriptParser::FunctionCallContext* context) override;
		std::any visitAdditive(GIScriptParser::AdditiveContext* context) override;
		std::any visitMultiplicative(GIScriptParser::MultiplicativeContext* context) override;
		std::any visitShift(GIScriptParser::ShiftContext* context) override;
		std::any visitRelational(GIScriptParser::RelationalContext* context) override;
		std::any visitEquality(GIScriptParser::EqualityContext* context) override;
		std::any visitAnd(GIScriptParser::AndContext* context) override;
		std::any visitXor(GIScriptParser::XorContext* context) override;
		std::any visitOr(GIScriptParser::OrContext* context) override;
		std::any visitLogicalAnd(GIScriptParser::LogicalAndContext* context) override;
		std::any visitLogicalOr(GIScriptParser::LogicalOrContext* context) override;
		std::any visitConditional(GIScriptParser::ConditionalContext* context) override;
		std::any visitUnary(GIScriptParser::UnaryContext* context) override;
		std::any visitAssignment(GIScriptParser::AssignmentContext* context) override;
		std::any visitExpr(GIScriptParser::ExprContext* context) override;
		std::any visitParenExpression(GIScriptParser::ParenExpressionContext* context) override;
		std::any visitCast(GIScriptParser::CastContext* context) override;

		std::any visitInitializer(GIScriptParser::InitializerContext* context) override;
		std::any visitInitializerList(GIScriptParser::InitializerListContext* context) override;
	public:
		Parser();
		std::unique_ptr<Ugc::Script::ASTNode> Release();
	};
}
