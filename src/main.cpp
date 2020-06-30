#include <ASTBuilder.hpp>
#include <Visitor.hpp>
#include <DumpVisitor.hpp>

#include <ostream>
#include <sstream>
constexpr std::string_view script =
R"__(
cmake_minimum_required(VERSION 3.12)
   project (CMakeAST LANGUAGES C CXX) # end line comment 



       set (VaR VaLuE)

if (VaR)
set(Var FALSE)
else()
set(Var TRUE)
endif()
)__";


/**
 * @brief Given an AST, modifies all the nodes contexts to be relative to the previous node.
 * @details First command argument node will be relative to the command invocation parenthesis.
 */
class MakeContextVisitor : public Visitor
{
public:
  enum class Type
  {
    Relative,
    Absolute
  };
  Type m_type;

  MakeContextVisitor(Type i_type)
    : Visitor()
    , m_type(i_type)
  {
    m_currentContextStack.emplace_back(1, 1);
  }

  Context& GetCurrentContext()
  {
    return m_currentContextStack.back();
  }

  void SetCurrentContext(Context i_context)
  {
    m_currentContextStack.back() = i_context;
  }

  void PushContext()
  {
    if (m_type == Type::Absolute)
    {
      if (m_currentContextStack.empty())
      {
        m_currentContextStack.emplace_back(1, 1);
      }
      else
      {
        m_currentContextStack.push_back(m_currentContextStack.back());
      }
    }
    else
    {
      if (m_currentContextStack.empty())
      {
        m_currentContextStack.emplace_back(0, 0);
      }
      else
      {
        m_currentContextStack.push_back(m_currentContextStack.back());
      }
    }
  }

  void PopContext()
  {
    m_currentContextStack.pop_back();
  }

  void OffsetRange(Range& i_range)
  {
    auto currentContet = this->GetCurrentContext();
    bool shouldUpdateColumn = (i_range.begin.line == currentContet.line);
    if (m_type == Type::Absolute)
    {
      i_range.begin.line += currentContet.line;
      if (shouldUpdateColumn)
      {
        i_range.begin.column += currentContet.column;
      }
      else
      {
        i_range.begin.column = currentContet.column;
      }
      i_range.end.line += currentContet.line;
      if (shouldUpdateColumn)
      {
        i_range.end.column = currentContet.column;
      }
    }
    else
    {
      auto originalRange = i_range;
      assert(i_range.begin.line >= currentContet.line);
      i_range.begin.line -= currentContet.line;
      if (shouldUpdateColumn)
      {
        assert(i_range.begin.column >= currentContet.column);
        i_range.begin.column -= currentContet.column;
      }

      assert(i_range.end.line >= currentContet.line);
      i_range.end.line -= currentContet.line;
      if (shouldUpdateColumn)
      {
        assert(i_range.end.column >= currentContet.column);
        i_range.end.column -= currentContet.column;
      }
    }
  }

  void Visit(BasicNode& i_node) override
  {
    // We need to keep the original node range when visiting children,
    // so the logic is done in the AfterVistChildren callback.
  }

  void BeforeVisitChildren(BasicNode& i_node) override
  {
    auto originalContext = this->GetCurrentContext();
    this->PushContext();

    // Set current context to begin in BeforeVisitChildren
    if (m_type == Type::Relative)
    {
      this->SetCurrentContext(i_node.GetRange().begin);
    }
    else
    {
      auto newContext = originalContext;
      newContext.line += i_node.GetRange().begin.line;
      newContext.column += i_node.GetRange().begin.column;
      this->SetCurrentContext(newContext);
    }
  }

  void AfterVisitChildren(BasicNode& i_node) override
  {
    this->PopContext();

    // Update the parent range
    if (m_type == Type::Absolute)
    {
      this->OffsetRange(i_node.GetRange());
      this->SetCurrentContext(i_node.GetRange().end);
    }
    else
    {
      auto nextContext = i_node.GetRange().end;
      this->OffsetRange(i_node.GetRange());
      this->SetCurrentContext(nextContext);
    }
  }

private:
  std::vector<Context> m_currentContextStack;
};

class FormatVisitor : public Visitor
{
public:
  void BeforeVisitChildren(BasicNode& i_node) override
  {
    if (i_node.GetChildren().empty())
    {
      return;
    }
    m_childOffset.push_back(Context{ 0,0 });
  }

  void AfterVisitChildren(BasicNode& i_node) override
  {
    if (i_node.GetChildren().empty())
    {
      return;
    }
    auto offset = m_childOffset.back();
    m_childOffset.pop_back();
    i_node.GetRange().end = i_node.GetRange().begin;
    i_node.GetRange().end.line += offset.line;
    i_node.GetRange().end.column += offset.column;
  }


  void BeforeVisitChildren(Node<BasicNode::Type::CommandInvocation>& i_node) override
  {
    this->Visitor::BeforeVisitChildren(i_node);

    if (i_node.GetCommandName() == "endif" ||
      i_node.GetCommandName() == "else" ||
      i_node.GetCommandName() == "elseif")
    {
      m_indentation -= 2;
    }
    if (m_needsNewLine)
    {
      i_node.GetRange().begin = Context{ 1, m_indentation };
      m_needsNewLine = false;
    }
    else
    {
      i_node.GetRange().begin = Context{ 0, m_indentation };
    }
    i_node.GetRange().end.line = i_node.GetRange().begin.line;
    i_node.GetRange().end.column = i_node.GetRange().begin.column + i_node.GetCommandName().size() + 1;
  }

  void AfterVisitChildren(Node<BasicNode::Type::CommandInvocation>& i_node) override
  {
    if (i_node.GetCommandName() == "if" ||
      i_node.GetCommandName() == "else" ||
      i_node.GetCommandName() == "elseif")
    {
      m_indentation += 2;
    }
    m_needsNewLine = true;

    this->Visitor::AfterVisitChildren(i_node);

    i_node.GetRange().end.column += i_node.GetCommandName().size();
  }

  void Visit(BasicNode& i_node) override
  {
    auto& range = i_node.GetRange();
    auto rangeOffset = Context(range.end.line - range.begin.line, range.end.column - range.begin.column);
    range.begin = Context(0, 1);
    range.end.line = range.begin.line + rangeOffset.line;
    range.end.column = range.begin.column + rangeOffset.column;

    if (!m_childOffset.empty())
    {
      m_childOffset.back().line += range.end.line;
      m_childOffset.back().column += range.end.column;
    }
  }

private:
  size_t m_indentation = 0;
  bool m_needsNewLine = false;
  std::vector<Context> m_childOffset;
};

#include <iostream>
class GraphvizVisitor : public Visitor
{
private:
  std::ostream& m_os;
public:
  //-----------------------------------------------------------------------------
  GraphvizVisitor(std::ostream& i_os)
    : Visitor()
    , m_os(i_os)
  {
    m_os << "digraph G {\n";
  }

  ~GraphvizVisitor()
  {
    m_os << "}\n";
  }

  void Visit(BasicNode& i_node) override
  {
    m_os << "\"" << &i_node << "\"" << R"__([label="Type=)__";
    switch (i_node.GetType())
    {
    case BasicNode::Type::CommandInvocation:
      m_os << "CommandInvocation";
      break;
    case BasicNode::Type::Argument:
      m_os << "Argument";
      break;
    case BasicNode::Type::Arguments:
      m_os << "Arguments";
      break;
    case BasicNode::Type::BracketComment:
      m_os << "BracketComment";
      break;
    case BasicNode::Type::Comment:
      m_os << "Comment";
      break;
    case BasicNode::Type::File:
      m_os << "File";
      break;
    }
    m_os << "\\nRange=[{" << i_node.GetRange().begin.line << ", " << i_node.GetRange().begin.column << "},{" << i_node.GetRange().end.line << ", " << i_node.GetRange().end.column << "}]";

    switch (i_node.GetType())
    {
    case BasicNode::Type::CommandInvocation:
    {
      auto& ci = dynamic_cast<Node<BasicNode::Type::CommandInvocation>&>(i_node);
      m_os << "\\nCommandName=\\\"" << ci.GetCommandName() << "\\\"";
      break;
    }
    case BasicNode::Type::Argument:
    {
      auto& arg = dynamic_cast<Node<BasicNode::Type::Argument>&>(i_node);
      m_os << "\\nArgumentValue=\\\"" << arg.GetValue() << "\\\"";
      break;
    }
    }
    m_os << "\"];\n";
  }

  void BeforeVisitChildren(BasicNode& i_node) override
  {
    for (auto& child : i_node.GetChildren())
    {
      m_os << "\"" << &i_node << "\" -> \"" << &(*child) << "\"\n";
    }
  }
};


void Dump(const BasicNode::PolymorphicNode& i_ast)
{
  i_ast->Accept(DumpVisitor{ std::cout });
}
void Format(BasicNode::PolymorphicNode& i_ast)
{
  i_ast->Accept(FormatVisitor{ });
}
void MakeRelative(BasicNode::PolymorphicNode& i_ast)
{
  i_ast->Accept(MakeContextVisitor{ MakeContextVisitor::Type::Relative });
}
void MakeAbsolute(BasicNode::PolymorphicNode& i_ast)
{
  i_ast->Accept(MakeContextVisitor{ MakeContextVisitor::Type::Absolute });
}

// TODO: this visitor should be const !
void GenerateGraph(BasicNode::PolymorphicNode& i_ast)
{
  i_ast->Accept(GraphvizVisitor{ std::cout });
}

int main()
{
  auto ast = ASTBuilder::Build(script);
  //GenerateGraph(ast);
  //Dump(ast);

  MakeRelative(ast);
  //GenerateGraph(ast);
  MakeAbsolute(ast);
  GenerateGraph(ast);

  Dump(ast);
}
