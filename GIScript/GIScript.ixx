module;
#define EXPORT __declspec(dllexport)
export module GIScript;

import std;

export namespace Ugc::Script
{
	struct ASTVisitor;

	class ASTNode
	{
	public:
		ASTNode();
		virtual void Visit(ASTVisitor& visitor) = 0;
		virtual ~ASTNode() = 0;
	};

	class DeclarationNode : public ASTNode
	{
	public:
		DeclarationNode();
	};

	class FunctionNode;

	class RootNode : public ASTNode
	{
		std::vector<std::unique_ptr<DeclarationNode>> declarations;
		std::vector<std::unique_ptr<FunctionNode>> global_functions;
	public:
		explicit RootNode(std::vector<std::unique_ptr<DeclarationNode>> declarations, std::vector<std::unique_ptr<FunctionNode>> global_functions);
		void Visit(ASTVisitor& visitor) override;

		std::vector<std::unique_ptr<FunctionNode>> GlobalFunctions() { return std::move(global_functions); }
	};

	class StatementNode : public ASTNode
	{
	public:
		StatementNode();
	};

	class NopStatement final : public StatementNode
	{
	public:
		NopStatement();
		void Visit(ASTVisitor& visitor) override;
	};

	class BreakStatement final : public StatementNode
	{
	public:
		BreakStatement();
		void Visit(ASTVisitor& visitor) override;
	};

	class ExpressionNode : public ASTNode
	{
		void Visit(ASTVisitor& visitor) final;
	public:
		ExpressionNode();

		virtual std::any Eval(ASTVisitor& visitor) = 0;
	};

	class BlockNode : public StatementNode
	{
		friend FunctionNode;
		std::vector<std::unique_ptr<StatementNode>> statements;
	public:
		explicit BlockNode(std::vector<std::unique_ptr<StatementNode>> statements);
		void Visit(ASTVisitor& visitor) override;
	};

	enum class GuidEx
	{
		Entity,
		Prefab,
		Configuration,
		Faction
	};

	struct VarType
	{
		enum Type
		{
			Unknown,
			Int,
			Float,
			String,
			Bool,
			Entity,
			Vec,
			Guid,
			List,
			Map,
			Tuple,
			Function
		};

		Type type;
		std::any extra;

		EXPORT bool operator==(const VarType& other) const;
	};

	struct MapEx
	{
		VarType key;
		VarType value;
	};

	class Variable
	{
		std::string id;
		VarType type;
		std::unique_ptr<ExpressionNode> value;
	public:
		Variable(const std::string& id, VarType type, std::unique_ptr<ExpressionNode> value = nullptr);

		const std::string& Id() const { return id; }
		const VarType& Type() const { return type; }
		ExpressionNode* Value() const { return value.get(); }
	};

	class EventNode : public DeclarationNode
	{
		std::string event;
		std::vector<Variable> parameters;
		BlockNode body;
	public:
		EventNode(const std::string& event, std::vector<Variable> parameters, BlockNode body);
		void Visit(ASTVisitor& visitor) override;
	};

	class FunctionNode : public DeclarationNode
	{
		std::string name;
		std::vector<Variable> parameters;
		std::optional<VarType> ret;
		BlockNode body;
	public:
		FunctionNode(const std::string& name, std::optional<VarType> ret, std::vector<Variable> parameters, BlockNode body);
		void Visit(ASTVisitor& visitor) override;

		std::string Name() const { return name; }
		const std::vector<Variable>& Parameters() const { return parameters; }
		std::optional<VarType> Ret() const { return ret; }
		void VisitBody(ASTVisitor& visitor) { body.Visit(visitor); }
	};

	class Return : public StatementNode
	{
		std::unique_ptr<ExpressionNode> expr;
	public:
		explicit Return(std::unique_ptr<ExpressionNode> expr = nullptr);
		void Visit(ASTVisitor& visitor) override;
	};

	class VarDef : public StatementNode
	{
		std::vector<Variable> vars;
	public:
		explicit VarDef(std::vector<Variable> vars);
		void Visit(ASTVisitor& visitor) override;
	};

	class ExprStatement : public StatementNode
	{
		std::unique_ptr<ExpressionNode> expr;
	public:
		explicit ExprStatement(std::unique_ptr<ExpressionNode> expr);
		void Visit(ASTVisitor& visitor) override;
	};

	class IfStatement : public StatementNode
	{
		std::unique_ptr<ExpressionNode> condition;
		std::unique_ptr<StatementNode> then;
		std::unique_ptr<StatementNode> otherwise;
	public:
		IfStatement(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<StatementNode> then, std::unique_ptr<StatementNode> otherwise = nullptr);
		void Visit(ASTVisitor& visitor) override;

		enum Phase
		{
			Start,
			Else,
			End
		};
	};

	class SwitchStatement;

	class CaseNode : public ASTNode
	{
		friend SwitchStatement;
		std::unique_ptr<ExpressionNode> literal;
		std::vector<std::unique_ptr<StatementNode>> statements;

		void Visit(ASTVisitor& visitor) override;
	public:
		CaseNode(std::unique_ptr<ExpressionNode> literal, std::vector<std::unique_ptr<StatementNode>> statements);
	};

	class SwitchStatement : public StatementNode
	{
		std::unique_ptr<ExpressionNode> expr;
		std::vector<std::unique_ptr<CaseNode>> cases;
		std::unique_ptr<CaseNode> default_case;
	public:
		SwitchStatement(std::unique_ptr<ExpressionNode> expr, std::vector<std::unique_ptr<CaseNode>> cases, std::unique_ptr<CaseNode> default_case = nullptr);
		void Visit(ASTVisitor& visitor) override;
	};

	class WhileStatement : public StatementNode
	{
		std::unique_ptr<ExpressionNode> expr;
		std::unique_ptr<StatementNode> body;
	public:
		explicit WhileStatement(std::unique_ptr<ExpressionNode> expr, std::unique_ptr<StatementNode> body);
		void Visit(ASTVisitor& visitor) override;
	};

	class ForStatement : public StatementNode
	{
		std::unique_ptr<StatementNode> init;
		std::unique_ptr<ExpressionNode> condition;
		std::unique_ptr<StatementNode> iteration;
		std::unique_ptr<StatementNode> body;
	public:
		explicit ForStatement(std::unique_ptr<StatementNode> init, std::unique_ptr<ExpressionNode> condition, std::unique_ptr<StatementNode> iteration, std::unique_ptr<StatementNode> body);
		void Visit(ASTVisitor& visitor) override;
	};

	class ForEachStatement : public StatementNode
	{
		VarType type;
		std::string def;
		std::unique_ptr<ExpressionNode> iterable;
		std::unique_ptr<StatementNode> body;
	public:
		explicit ForEachStatement(VarType type, const std::string& def, std::unique_ptr<ExpressionNode> iterable, std::unique_ptr<StatementNode> body);
		void Visit(ASTVisitor& visitor) override;
	};

	class Literal : public ExpressionNode
	{
	public:
		enum Type
		{
			Unknown,
			Int,
			Float,
			Bool,
			String,
			Null
		};
	private:
		Type type;
		std::any value;
	public:
		Literal(Type type, std::any value);
		std::any Eval(ASTVisitor& visitor) override;
		Literal BitwiseNOT() const;
		Literal LogicalNOT() const;
		Literal Negate() const;
	};

	class Identifier : public ExpressionNode
	{
		std::string id;
	public:
		explicit Identifier(const std::string& id);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class CallExpr : public ExpressionNode
	{
		std::unique_ptr<ExpressionNode> expr;
		std::vector<std::unique_ptr<ExpressionNode>> args;
		std::optional<VarType> type;
	public:
		CallExpr(std::unique_ptr<ExpressionNode> expr, std::vector<std::unique_ptr<ExpressionNode>> args, std::optional<VarType> type = {});
		std::any Eval(ASTVisitor& visitor) override;
	};

	class Increment : public ExpressionNode
	{
		std::unique_ptr<ExpressionNode> expr;
		bool pre;
		bool inv;
	public:
		Increment(std::unique_ptr<ExpressionNode> expr, bool inv, bool pre);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class MemberExpr : public ExpressionNode
	{
		std::unique_ptr<ExpressionNode> expr;
		std::unique_ptr<ExpressionNode> member;
		std::optional<VarType> type;
	public:
		MemberExpr(std::unique_ptr<ExpressionNode> expr, std::unique_ptr<ExpressionNode> member, std::optional<VarType> type = {});
		std::any Eval(ASTVisitor& visitor) override;
	};

	class Assignment : public ExpressionNode
	{
	public:
		enum Op
		{
			Normal,
			Add,
			Sub,
			Mul,
			Div
		};
	private:
		std::unique_ptr<ExpressionNode> ref;
		std::unique_ptr<ExpressionNode> expr;
		Op op;
	public:
		Assignment(std::unique_ptr<ExpressionNode> ref, std::unique_ptr<ExpressionNode> expr, Op op);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class UnaryExpr : public ExpressionNode
	{
	public:
		enum Op
		{
			Negate,
			LogicalNOT,
			BitwiseNOT
		};
	private:
		Op op;
		std::unique_ptr<ExpressionNode> expr;
	public:
		UnaryExpr(Op op, std::unique_ptr<ExpressionNode> expr);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class BinaryExpr : public ExpressionNode
	{
	public:
		enum Op
		{
			Add,
			Sub,
			Mul,
			Div,
			Mod,
			ShL,
			ShR,
			ShA,
			LT,
			GT,
			LE,
			GE,
			EQ,
			NE,
			AND,
			XOR,
			OR,
			LogAND,
			LogXOR,
			LogOR
		};
	private:
		Op op;
		std::unique_ptr<ExpressionNode> l, r;
	public:
		BinaryExpr(Op op, std::unique_ptr<ExpressionNode> l, std::unique_ptr<ExpressionNode> r);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class TernaryExpr : public ExpressionNode
	{
		std::unique_ptr<ExpressionNode> e1;
		std::unique_ptr<ExpressionNode> e2;
		std::unique_ptr<ExpressionNode> e3;
	public:
		TernaryExpr(std::unique_ptr<ExpressionNode> e1, std::unique_ptr<ExpressionNode> e2, std::unique_ptr<ExpressionNode> e3);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class ChainExpr : public ExpressionNode
	{
		std::vector<std::unique_ptr<ExpressionNode>> elements;
	public:
		explicit ChainExpr(std::vector<std::unique_ptr<ExpressionNode>> elements);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class CastExpr : public ExpressionNode
	{
		VarType type;
		std::unique_ptr<ExpressionNode> expr;
	public:
		CastExpr(VarType type, std::unique_ptr<ExpressionNode> expr);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class ConstructExpr : public ExpressionNode
	{
		VarType type;
		std::vector<std::unique_ptr<ExpressionNode>> initializers;
	public:
		ConstructExpr(VarType type, std::vector<std::unique_ptr<ExpressionNode>> initializers);
		std::any Eval(ASTVisitor& visitor) override;
	};

	class InitializerList : public ExpressionNode
	{
		std::vector<std::unique_ptr<ExpressionNode>> initializers;
	public:
		explicit InitializerList(std::vector<std::unique_ptr<ExpressionNode>> initializers);
		std::any Eval(ASTVisitor& visitor) override;
	};

	struct LocalVar
	{
		VarType type;
		std::any content;
	};

	class ScopeTable
	{
		struct Scope : std::unordered_map<std::string, std::unique_ptr<LocalVar>>
		{
			using Base = std::unordered_map<std::string, std::unique_ptr<LocalVar>>;
			using Base::Base;

			Scope(Scope&& other) noexcept : Base(std::move(other)) {}
			Scope& operator=(Scope&& other) noexcept
			{
				Base::operator=(std::move(other));
				return *this;
			}

			Scope(const Scope&) = delete;
			Scope& operator=(const Scope&) = delete;
		};

		std::vector<Scope> scopes;
	public:
		void enter() { scopes.emplace_back(); }
		void exit() { scopes.pop_back(); }

		void add(const std::string& name, std::unique_ptr<LocalVar> value) { scopes.back()[name] = std::move(value); }

		bool contains(const std::string& name) const { return scopes.back().contains(name); }

		LocalVar* find(const std::string& name)
		{
			for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
			{
				auto found = it->find(name);
				if (found != it->end()) return found->second.get();
			}
			return nullptr;
		}
	};

	struct ASTVisitor
	{
		ScopeTable scope;

		virtual void VisitEvent(const std::string& event, const std::vector<Variable>& parameters) {}
		virtual void VisitFunction(const std::string& name, std::optional<VarType> ret, const std::vector<Variable>& parameters) {}
		virtual void VisitVarDef(const std::string& id, VarType type, const std::any& value) {}
		virtual void VisitExprStatement(const std::any& value) {}
		virtual void VisitIfStatement(IfStatement::Phase phase, std::any& value) {}
		virtual void VisitSwitchStatement(int count, std::any& value, bool end) {}
		virtual void VisitCase(const std::any& literal, std::any& value) {}
		virtual void VisitWhile(std::any& value, bool end) {}
		virtual void VisitFor(std::any& value, bool end) {}
		virtual void VisitForEachStart(VarType type, const std::string& var, std::any& value) {}
		virtual void VisitForEachEnd(std::any& value) {}
		virtual void VisitBreak() {}
		virtual void VisitReturn(const std::any& value) {}
		virtual std::any VisitLiteral(Literal::Type type, const std::any& value) { return {}; }
		virtual std::any VisitAssignment(const std::any& ref, Assignment::Op op, const std::any& value) { return {}; }
		virtual std::any VisitCall(const std::any& value, const std::vector<std::any>& args, std::optional<VarType> type) { return {}; }
		virtual std::any VisitIdentifier(const std::string& id) { return {}; }
		virtual std::any VisitIncrement(const std::any& ref, bool inv, bool pre) { return {}; }
		virtual std::any VisitMemberAccess(const std::any& value, const std::any& member, std::optional<VarType> type) { return {}; }
		virtual std::any VisitUnary(UnaryExpr::Op op, const std::any& value) { return {}; }
		virtual std::any VisitBinary(BinaryExpr::Op op, const std::any& l, const std::any& r) { return {}; }
		virtual std::any VisitTernary(const std::any& e1, const std::any& e2, const std::any& e3) { return {}; }
		virtual std::any VisitCast(VarType type, const std::any& value) { return {}; }
		virtual std::any VisitConstruct(VarType type, const std::vector<std::any>& args) { return {}; }
		virtual std::any VisitInitializerList(const std::vector<std::any>& values) { return {}; }
		virtual VarType TypeInference(const std::any& value) { return {}; }
		virtual ~ASTVisitor() {}
	};

	EXPORT std::unique_ptr<ASTNode> Parse(const std::string& code);
}

using namespace Ugc::Script;

export template<>
struct std::hash<VarType>
{
	size_t operator()(const VarType& type) const noexcept
	{
		auto h = hash<int>{};
		size_t seed = h(type.type);
		if (type.type == VarType::Guid) seed ^= h((int)std::any_cast<GuidEx>(type.extra));
		else if (type.type == VarType::List) seed ^= h(hash{}(std::any_cast<const VarType&>(type.extra)));
		else if (type.type == VarType::Map)
		{
			hash h1{};
			auto& [key, value] = std::any_cast<const MapEx&>(type.extra);
			seed ^= h1(key);
			seed ^= h1(value);
		}
		return seed;
	}
};
