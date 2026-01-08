module script;

using namespace GI::Script;
using namespace Ugc::Script;

static VarType MakeType(const GIScriptParser::BuiltinTypeContext* context)
{
	static const std::unordered_map<std::string, VarType::Type> map
	{
	{ "int", VarType::Int },
	{ "float", VarType::Float },
	{ "string", VarType::String },
	{ "bool", VarType::Bool },
	{ "entity", VarType::Entity },
	{ "vec", VarType::Vec }
	};
	if (auto it = map.find(context->children[0]->getText()); it != map.end()) return VarType{ it->second, {} };
	if (context->children[0]->getText() == "guid")
	{
		static const std::unordered_map<std::string, GuidEx> con
		{
		{ "entity", GuidEx::Entity },
		{ "prefab", GuidEx::Prefab },
		{ "cfg", GuidEx::Configuration },
		{ "faction", GuidEx::Faction }
		};
		return VarType{ VarType::Guid, con.at(context->con->getText()) };
	}
	throw std::runtime_error("Unknown type");
}

static VarType MakeType(const GIScriptParser::GenericTypeContext* context)
{
	if (context->children[0]->getText() == "list")
	{
		auto t = MakeType(context->t);
		return VarType{ VarType::List, t };
	}
	if (context->children[0]->getText() == "map")
	{
		auto k = MakeType(context->k);
		auto v = MakeType(context->v);
		return VarType{ VarType::Map, MapEx{ k, v } };
	}
	throw std::runtime_error("Unknown type");
}

static VarType MakeType(GIScriptParser::SingleTypeContext* context)
{
	if (context->builtinType()) return MakeType(context->builtinType());
	return MakeType(context->genericType());
}

static VarType MakeType(GIScriptParser::TypeContext* context)
{
	if (context->singleType()) return MakeType(context->singleType());
	std::vector<VarType> types;
	for (auto t : context->tuple()->singleType()) types.emplace_back(MakeType(t));
	return VarType{ VarType::Tuple, types };
}

template<typename T>
static std::unique_ptr<T> MakeUnique(const std::any& expr)
{
	return std::unique_ptr<T>(std::any_cast<T*>(expr));
}

static std::any Wrap(StatementNode* node)
{
	return node;
}

static std::any Wrap(ExpressionNode* node)
{
	return node;
}

static std::string MakeString(const std::string& text)
{
	static const std::regex r1(R"(\\")"), r2(R"(\\n)");
	auto str = std::regex_replace(text.substr(1, text.length() - 2), r1, R"(")");
	return std::regex_replace(str, r2, "\n");
}

std::any Parser::visitEvent(GIScriptParser::EventContext* context)
{
	std::vector<Variable> parameters;
	if (context->parameterList())
	{
		for (auto p : context->parameterList()->parameter())
		{
			parameters.emplace_back(p->ID()->getText(), MakeType(p->type()));
		}
	}
	declarations.emplace_back(std::make_unique<EventNode>(context->ID()->getText(), std::move(parameters), std::move(*std::unique_ptr<BlockNode>((BlockNode*)std::any_cast<StatementNode*>(visitBlock(context->block()))))));
	return {};
}

std::any Parser::visitFunction(GIScriptParser::FunctionContext* context)
{
	std::vector<Variable> parameters;
	std::optional<VarType> ret;
	if (context->functionSign()->type()) ret = MakeType(context->functionSign()->type());
	if (context->functionSign()->parameterList())
	{
		for (auto p : context->functionSign()->parameterList()->parameter())
		{
			parameters.emplace_back(p->ID()->getText(), MakeType(p->type()));
		}
	}
	if (context->children[0]->getText() == "global") global_functions.emplace_back(std::make_unique<FunctionNode>(context->functionSign()->ID()->getText(), std::move(ret), std::move(parameters), std::move(*std::unique_ptr<BlockNode>((BlockNode*)std::any_cast<StatementNode*>(visitBlock(context->block()))))));
	else declarations.emplace_back(std::make_unique<FunctionNode>(context->functionSign()->ID()->getText(), std::move(ret), std::move(parameters), std::move(*std::unique_ptr<BlockNode>((BlockNode*)std::any_cast<StatementNode*>(visitBlock(context->block()))))));
	return {};
}

std::any Parser::visitBlock(GIScriptParser::BlockContext* context)
{
	std::vector<std::unique_ptr<StatementNode>> sts;
	for (auto s : context->statement()) sts.emplace_back(MakeUnique<StatementNode>(visitStatement(s)));
	return Wrap(new BlockNode(std::move(sts)));
}

std::any Parser::visitStatement(GIScriptParser::StatementContext* context)
{
	if (context->expr()) return Wrap(new ExprStatement(MakeUnique<ExpressionNode>(visit(context->expr()))));
	if (context->children[0]->getText() == ";") return Wrap(new NopStatement);
	if (context->children[0]->getText() == "break") return Wrap(new BreakStatement);
	return visit(context->children[0]);
}

std::any Parser::visitVarDef(GIScriptParser::VarDefContext* context)
{
	auto type = context->type() ? MakeType(context->type()) : VarType{};
	std::vector<Variable> vars;
	for (auto v : context->varInit())
	{
		std::unique_ptr<ExpressionNode> value;
		if (v->initializer()) value = MakeUnique<ExpressionNode>(visit(v->initializer()));
		vars.emplace_back(v->ID()->getText(), type, std::move(value));
	}
	return Wrap(new VarDef(std::move(vars)));
}

std::any Parser::visitIf(GIScriptParser::IfContext* context)
{
	return Wrap(new IfStatement(
		MakeUnique<ExpressionNode>(visit(context->expr())),
		MakeUnique<StatementNode>(visitStatement(context->then)),
		context->else_ ? MakeUnique<StatementNode>(visitStatement(context->else_)) : nullptr
	));
}

std::any Parser::visitSwitch(GIScriptParser::SwitchContext* context)
{
	std::vector<std::unique_ptr<CaseNode>> cases;
	for (auto c : context->case_()) cases.emplace_back(MakeUnique<CaseNode>(visitCase(c)));
	if (context->default_()) return Wrap(new SwitchStatement(MakeUnique<ExpressionNode>(visit(context->expr())), std::move(cases), MakeUnique<CaseNode>(visitDefault(context->default_()))));
	return Wrap(new SwitchStatement(MakeUnique<ExpressionNode>(visit(context->expr())), std::move(cases)));
}

std::any Parser::visitCase(GIScriptParser::CaseContext* context)
{
	std::vector<std::unique_ptr<StatementNode>> sts;
	for (auto s : context->statement()) sts.emplace_back(MakeUnique<StatementNode>(visitStatement(s)));
	std::unique_ptr<ExpressionNode> literal;
	if (context->STRING_DEF()) literal = std::make_unique<Literal>(Literal::String, MakeString(context->STRING_DEF()->getText()));
	else literal = std::make_unique<Literal>(Literal::Int, std::stoll(context->INT_DEF()->getText()));
	return new CaseNode(std::move(literal), std::move(sts));
}

std::any Parser::visitDefault(GIScriptParser::DefaultContext* context)
{
	std::vector<std::unique_ptr<StatementNode>> sts;
	for (auto s : context->statement()) sts.emplace_back(MakeUnique<StatementNode>(visitStatement(s)));
	return new CaseNode(nullptr, std::move(sts));
}

std::any Parser::visitWhile(GIScriptParser::WhileContext* context)
{
	return Wrap(new WhileStatement(MakeUnique<ExpressionNode>(visit(context->expr())), MakeUnique<StatementNode>(visit(context->statement()))));
}

std::any Parser::visitFor(GIScriptParser::ForContext* context)
{
	std::unique_ptr<StatementNode> init;
	std::unique_ptr<ExpressionNode> condition;
	std::unique_ptr<StatementNode> iteration;
	if (auto fi = context->forInit())
	{
		if (fi->varDef()) init = MakeUnique<StatementNode>(visit(fi->varDef()));
		else init = std::make_unique<ExprStatement>(MakeUnique<ExpressionNode>(visit(fi->expr())));
	}
	if (context->cond) condition = MakeUnique<ExpressionNode>(visit(context->cond));
	if (context->it) iteration = std::make_unique<ExprStatement>(MakeUnique<ExpressionNode>(visit(context->it)));
	return Wrap(new ForStatement(std::move(init), std::move(condition), std::move(iteration), MakeUnique<StatementNode>(visit(context->statement()))));
}

std::any Parser::visitForEach(GIScriptParser::ForEachContext* context)
{
	VarType type{};
	if (context->type()) type = MakeType(context->type());
	return Wrap(new ForEachStatement(std::move(type), context->ID()->getText(), MakeUnique<ExpressionNode>(visit(context->assignment())), MakeUnique<StatementNode>(visit(context->statement()))));
}

std::any Parser::visitReturn(GIScriptParser::ReturnContext* context)
{
	if (context->expr()) return Wrap(new Return(MakeUnique<ExpressionNode>(visit(context->expr()))));
	return Wrap(new Return());
}

std::any Parser::visitIntegerLiteral(GIScriptParser::IntegerLiteralContext* context)
{
	return Wrap(new Literal(Literal::Int, std::stoll(context->INT_DEF()->getText())));
}

std::any Parser::visitFloatLiteral(GIScriptParser::FloatLiteralContext* context)
{
	auto parse = [](std::string str)
		{
			auto value = 0.0f;
			if (auto result = std::from_chars(str.data(), str.data() + str.size(), value); result.ec == std::errc::invalid_argument)
			{
				if (str.empty()) return 0.0f;
				if (str[0] == '.') str = "0" + str;
				if (str.back() == '.') str = str + "0";
				result = std::from_chars(str.data(), str.data() + str.size(), value);
				if (result.ec != std::errc()) return 0.0f;
			}
			else if (result.ec == std::errc::result_out_of_range) return 0.0f;
			return value;
		};
	return Wrap(new Literal(Literal::Float, parse(context->FLOAT_DEF()->getText())));
}

std::any Parser::visitStringLiteral(GIScriptParser::StringLiteralContext* context)
{
	return Wrap(new Literal(Literal::String, MakeString(context->STRING_DEF()->getText())));
}

std::any Parser::visitIdentifierLiteral(GIScriptParser::IdentifierLiteralContext* context)
{
	return Wrap(new Identifier(context->ID()->getText()));
}

std::any Parser::visitKeywordLiteral(GIScriptParser::KeywordLiteralContext* context)
{
	auto text = context->getText();
	if (text == "true") return Wrap(new Literal(Literal::Bool, true));
	if (text == "false") return Wrap(new Literal(Literal::Bool, false));
	if (text == "null") return Wrap(new Literal(Literal::Null, {}));
	return Wrap(new Identifier(text));
}

std::any Parser::visitTypeInitializer(GIScriptParser::TypeInitializerContext* context)
{
	std::vector<std::unique_ptr<ExpressionNode>> inits;
	for (auto init : context->initializerList()->initializer()) inits.emplace_back(MakeUnique<ExpressionNode>(visit(init)));
	return Wrap(new ConstructExpr(MakeType(context->singleType()), std::move(inits)));
}

std::any Parser::visitPostfix(GIScriptParser::PostfixContext* context)
{
	auto expr = std::any_cast<ExpressionNode*>(visit(context->primary()));
	for (int i = 1; i < context->children.size(); i++)
	{
		auto n = context->children[i];
		if (auto fc = dynamic_cast<GIScriptParser::FunctionCallContext*>(n))
		{
			std::vector<std::unique_ptr<ExpressionNode>> args;
			for (auto a : std::any_cast<std::vector<ExpressionNode*>>(visitFunctionCall(fc))) args.emplace_back(std::unique_ptr<ExpressionNode>(a));
			if (fc->type()) expr = new CallExpr(std::unique_ptr<ExpressionNode>(expr), std::move(args), MakeType(fc->type()));
			else expr = new CallExpr(std::unique_ptr<ExpressionNode>(expr), std::move(args));
		}
		else if (auto ma = dynamic_cast<GIScriptParser::MemberAccessContext*>(n))
		{
			std::optional<VarType> type;
			if (ma->singleType()) type = MakeType(ma->singleType());
			if (ma->ID()) expr = new MemberExpr(std::unique_ptr<ExpressionNode>(expr), std::make_unique<Literal>(Literal::String, ma->ID()->getText()), std::move(type));
			else expr = new MemberExpr(std::unique_ptr<ExpressionNode>(expr), MakeUnique<ExpressionNode>(visit(ma->expr())), std::move(type));
		}
		else if (auto inc = dynamic_cast<GIScriptParser::IncrementContext*>(n)) expr = new Increment(std::unique_ptr<ExpressionNode>(expr), inc->getText() == "--", false);
		else throw std::exception();
	}
	return expr;
}

std::any Parser::visitFunctionCall(GIScriptParser::FunctionCallContext* context)
{
	std::vector<ExpressionNode*> args;
	if (context->argumentList())
	{
		for (auto e : context->argumentList()->assignment())
		{
			args.emplace_back(std::any_cast<ExpressionNode*>(visit(e)));
		}
	}
	return std::move(args);
}

std::any Parser::visitAdditive(GIScriptParser::AdditiveContext* context)
{
	if (!context->additive()) return visit(context->multiplicative());
	static const std::unordered_map<std::string, BinaryExpr::Op> map
	{
	{ "+", BinaryExpr::Add },
	{ "-", BinaryExpr::Sub }
	};
	if (auto it = map.find(context->op->getText()); it != map.end()) return Wrap(new BinaryExpr(it->second, MakeUnique<ExpressionNode>(visit(context->additive())), MakeUnique<ExpressionNode>(visit(context->multiplicative()))));
	throw std::exception();
}

std::any Parser::visitMultiplicative(GIScriptParser::MultiplicativeContext* context)
{
	if (!context->multiplicative()) return visit(context->cast());
	static const std::unordered_map<std::string, BinaryExpr::Op> map
	{
	{ "*", BinaryExpr::Mul },
	{ "/", BinaryExpr::Div },
	{ "%", BinaryExpr::Mod }
	};
	if (auto it = map.find(context->op->getText()); it != map.end()) return Wrap(new BinaryExpr(it->second, MakeUnique<ExpressionNode>(visit(context->multiplicative())), MakeUnique<ExpressionNode>(visit(context->cast()))));
	throw std::exception();
}

std::any Parser::visitShift(GIScriptParser::ShiftContext* context)
{
	if (!context->shift()) return visit(context->additive());
	static const std::unordered_map<std::string, BinaryExpr::Op> map
	{
	{ "<<", BinaryExpr::ShL },
	{ ">>", BinaryExpr::ShA },
	{ ">>>", BinaryExpr::ShR },
	};
	if (auto it = map.find(context->op->getText()); it != map.end()) return Wrap(new BinaryExpr(it->second, MakeUnique<ExpressionNode>(visit(context->shift())), MakeUnique<ExpressionNode>(visit(context->additive()))));
	throw std::exception();
}

std::any Parser::visitRelational(GIScriptParser::RelationalContext* context)
{
	if (!context->relational()) return visit(context->shift());
	static const std::unordered_map<std::string, BinaryExpr::Op> map
	{
	{ "<", BinaryExpr::LT },
	{ ">", BinaryExpr::GT },
	{ "<=", BinaryExpr::LE },
	{ ">=", BinaryExpr::GE }
	};
	if (auto it = map.find(context->op->getText()); it != map.end()) return Wrap(new BinaryExpr(it->second, MakeUnique<ExpressionNode>(visit(context->relational())), MakeUnique<ExpressionNode>(visit(context->shift()))));
	throw std::exception();
}

std::any Parser::visitEquality(GIScriptParser::EqualityContext* context)
{
	if (!context->equality()) return visit(context->relational());
	if (context->op->getText() == "==") return Wrap(new BinaryExpr(BinaryExpr::EQ, MakeUnique<ExpressionNode>(visit(context->equality())), MakeUnique<ExpressionNode>(visit(context->relational()))));
	return Wrap(new BinaryExpr(BinaryExpr::NE, MakeUnique<ExpressionNode>(visit(context->equality())), MakeUnique<ExpressionNode>(visit(context->relational()))));
}

std::any Parser::visitAnd(GIScriptParser::AndContext* context)
{
	if (!context->and_()) return visit(context->equality());
	return Wrap(new BinaryExpr(BinaryExpr::AND, MakeUnique<ExpressionNode>(visit(context->and_())), MakeUnique<ExpressionNode>(visit(context->equality()))));
}

std::any Parser::visitXor(GIScriptParser::XorContext* context)
{
	if (!context->xor_()) return visit(context->and_());
	return Wrap(new BinaryExpr(BinaryExpr::XOR, MakeUnique<ExpressionNode>(visit(context->xor_())), MakeUnique<ExpressionNode>(visit(context->and_()))));
}

std::any Parser::visitOr(GIScriptParser::OrContext* context)
{
	if (!context->or_()) return visit(context->xor_());
	return Wrap(new BinaryExpr(BinaryExpr::OR, MakeUnique<ExpressionNode>(visit(context->or_())), MakeUnique<ExpressionNode>(visit(context->xor_()))));
}

std::any Parser::visitLogicalAnd(GIScriptParser::LogicalAndContext* context)
{
	if (!context->logicalAnd()) return visit(context->or_());
	return Wrap(new BinaryExpr(BinaryExpr::LogAND, MakeUnique<ExpressionNode>(visit(context->logicalAnd())), MakeUnique<ExpressionNode>(visit(context->or_()))));
}

std::any Parser::visitLogicalOr(GIScriptParser::LogicalOrContext* context)
{
	if (!context->logicalOr()) return visit(context->logicalAnd());
	return Wrap(new BinaryExpr(BinaryExpr::LogOR, MakeUnique<ExpressionNode>(visit(context->logicalOr())), MakeUnique<ExpressionNode>(visit(context->logicalAnd()))));
}

std::any Parser::visitConditional(GIScriptParser::ConditionalContext* context)
{
	if (!context->conditional()) return visit(context->logicalOr());
	auto cond = std::any_cast<ExpressionNode*>(visit(context->logicalOr()));
	auto then = std::any_cast<ExpressionNode*>(visit(context->expr()));
	auto other = std::any_cast<ExpressionNode*>(visit(context->conditional()));
	return Wrap(new TernaryExpr(std::unique_ptr<ExpressionNode>(cond), std::unique_ptr<ExpressionNode>(then), std::unique_ptr<ExpressionNode>(other)));
}

std::any Parser::visitUnary(GIScriptParser::UnaryContext* context)
{
	ExpressionNode* e;
	if (context->postfix()) e = std::any_cast<ExpressionNode*>(visit(context->postfix()));
	else if (context->primary()) e = std::any_cast<ExpressionNode*>(visit(context->primary()));
	else
	{
		auto expr = std::any_cast<ExpressionNode*>(visit(context->cast()));
		static const std::unordered_map<std::string, UnaryExpr::Op> map
		{
		{ "-", UnaryExpr::Negate },
		{ "!", UnaryExpr::LogicalNOT },
		{ "~", UnaryExpr::BitwiseNOT }
		};
		if (auto l = dynamic_cast<Literal*>(expr))
		{
			if (auto it = map.find(context->op->getText()); it != map.end())
			{
				switch (it->second)
				{
				case UnaryExpr::Negate:
					*l = l->Negate();
					break;
				case UnaryExpr::LogicalNOT:
					*l = l->LogicalNOT();
					break;
				case UnaryExpr::BitwiseNOT:
					*l = l->BitwiseNOT();
					break;
				}
				return expr;
			}
			throw std::exception();
		}
		if (auto it = map.find(context->op->getText()); it != map.end()) e = new UnaryExpr(it->second, std::unique_ptr<ExpressionNode>(expr));
		else throw std::exception();
	}
	for (auto i : context->increment()) e = new Increment(std::unique_ptr<ExpressionNode>(e), i->getText() == "--", true);
	return e;
}

std::any Parser::visitAssignment(GIScriptParser::AssignmentContext* context)
{
	if (context->conditional()) return visit(context->conditional());
	static const std::unordered_map<std::string, Assignment::Op> map
	{
	{ "=", Assignment::Normal },
	{ "+=", Assignment::Add },
	{ "-=", Assignment::Sub },
	{ "*=", Assignment::Mul },
	{ "/=", Assignment::Div }
	};
	return Wrap(new Assignment(MakeUnique<ExpressionNode>(visit(context->unary())), MakeUnique<ExpressionNode>(visit(context->initializer())), map.at(context->op->getText())));
}

std::any Parser::visitExpr(GIScriptParser::ExprContext* context)
{
	auto expr = context->assignment();
	if (expr.size() == 1) return visit(expr[0]);
	std::vector<std::unique_ptr<ExpressionNode>> exprs;
	for (auto e : expr) exprs.emplace_back(MakeUnique<ExpressionNode>(visit(e)));
	return Wrap(new ChainExpr(std::move(exprs)));
}

std::any Parser::visitParenExpression(GIScriptParser::ParenExpressionContext* context)
{
	return visit(context->expr());
}

std::any Parser::visitCast(GIScriptParser::CastContext* context)
{
	if (!context->cast()) return visit(context->unary());
	return Wrap(new CastExpr(MakeType(context->singleType()), MakeUnique<ExpressionNode>(visit(context->cast()))));
}

std::any Parser::visitInitializer(GIScriptParser::InitializerContext* context)
{
	if (context->assignment()) return visit(context->assignment());
	return visit(context->initializerList());
}

std::any Parser::visitInitializerList(GIScriptParser::InitializerListContext* context)
{
	std::vector<std::unique_ptr<ExpressionNode>> inits;
	for (auto init : context->initializer()) inits.emplace_back(MakeUnique<ExpressionNode>(visit(init)));
	return Wrap(new InitializerList(std::move(inits)));
}

Parser::Parser()
{
}

std::unique_ptr<ASTNode> Parser::Release()
{
	return std::make_unique<RootNode>(std::move(declarations), std::move(global_functions));
}
