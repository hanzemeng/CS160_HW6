#include "typecheck.hpp"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of do while loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
  classTable = new ClassTable();
  node->visit_children(this);
  if((*classTable).end() == (*classTable).find("Main"))
  {
    typeError(no_main_class);
  }
  if(0 != (*classTable)["Main"].membersSize)
  {
    typeError(main_class_members_present);
  }
  MethodTable* mainMethods = (*classTable)["Main"].methods;
  if((*mainMethods).end() == (*mainMethods).find("main"))
  {
    typeError(no_main_method);
  }
  if(bt_none != (*mainMethods)["main"].returnType.baseType || 0 != (*(*mainMethods)["main"].parameters).size())
  {
    typeError(main_method_incorrect_signature);
  }
}

void TypeCheck::visitClassNode(ClassNode* node) {
  //ClassInfo classInfo;
  currentClassName = node->identifier_1->name;
  (*classTable)[currentClassName].superClassName = node->identifier_2 ? node->identifier_2->name : "";
  if("" != (*classTable)[currentClassName].superClassName && (*classTable).end() == (*classTable).find((*classTable)[currentClassName].superClassName))
  {
    typeError(undefined_class);
  }

  currentMemberOffset = 0;
  currentVariableTable = new VariableTable();
  if (node->declaration_list) {
    for(std::list<DeclarationNode*>::iterator i = node->declaration_list->begin(); i != node->declaration_list->end(); i++) {
      DeclarationNode* currentDeclarationList = *i;
      currentDeclarationList->accept(this);
      if (currentDeclarationList->identifier_list) {
        for(std::list<IdentifierNode*>::iterator j = currentDeclarationList->identifier_list->begin(); j != currentDeclarationList->identifier_list->end(); j++) {
          IdentifierNode* currentIdentifier = *j;
          VariableInfo varInfo;
          varInfo.type.baseType = currentDeclarationList->type->basetype;
          varInfo.type.objectClassName = currentDeclarationList->type->objectClassName;
          varInfo.offset = currentMemberOffset;
          currentMemberOffset += 4;
          varInfo.size = 4; // always 4 according to typecheck.hpp 20, but what about object type ???
          (*currentVariableTable)[currentIdentifier->name] = varInfo;
        }
      }
    }
  }
  (*classTable)[currentClassName].members = currentVariableTable;
  (*classTable)[currentClassName].membersSize = currentMemberOffset;

  currentMethodTable = new MethodTable();
  if (node->method_list) {
    for(std::list<MethodNode*>::iterator iter = node->method_list->begin(); iter != node->method_list->end(); iter++) {
      (*iter)->accept(this);
    }
  }
  (*classTable)[currentClassName].methods = currentMethodTable;
  if((*((*classTable)[currentClassName]).methods).end() != (*((*classTable)[currentClassName]).methods).find(currentClassName) && 
    bt_none != (*((*classTable)[currentClassName]).methods)[currentClassName].returnType.baseType)
    {
      typeError(constructor_returns_type);
    }

  //(*classTable)[currentClassName] = classInfo;
}

void TypeCheck::visitMethodNode(MethodNode* node) {
  MethodInfo methodInfo;
  node->type->accept(this);
  methodInfo.returnType.baseType = node->type->basetype;
  methodInfo.returnType.objectClassName = node->type->objectClassName;

  currentLocalOffset = -4;
  currentVariableTable = new VariableTable();
  if (node->methodbody->declaration_list) {
    for(std::list<DeclarationNode*>::iterator i = node->methodbody->declaration_list->begin(); i != node->methodbody->declaration_list->end(); i++) {
      DeclarationNode* currentDeclarationList = *i;
      currentDeclarationList->accept(this);
      if (currentDeclarationList->identifier_list) {
        for(std::list<IdentifierNode*>::iterator j = currentDeclarationList->identifier_list->begin(); j != currentDeclarationList->identifier_list->end(); j++) {
          IdentifierNode* currentIdentifier = *j;
          VariableInfo varInfo;
          varInfo.type.baseType = currentDeclarationList->type->basetype;
          varInfo.type.objectClassName = currentDeclarationList->type->objectClassName;
          varInfo.offset = currentLocalOffset;
          currentLocalOffset -= 4;
          varInfo.size = 4; // always 4 according to typecheck.hpp 20, but what about object type ???
          (*currentVariableTable)[currentIdentifier->name] = varInfo;
        }
      }
    }
  }
  
  currentParameterOffset = 12;
  std::list<CompoundType>* parameters = new std::list<CompoundType>();
  if (node->parameter_list) {
    for(std::list<ParameterNode*>::iterator iter = node->parameter_list->begin(); iter != node->parameter_list->end(); iter++) {
      ParameterNode* currentParameter = *iter;
      currentParameter->accept(this);
      CompoundType temp;
      temp.baseType = currentParameter->type->basetype;
      temp.objectClassName = currentParameter->type->objectClassName;
      parameters->push_back(temp);
      VariableInfo varInfo;
      varInfo.type = temp;
      varInfo.offset = currentParameterOffset;
      currentParameterOffset += 4;
      varInfo.size = 4; // always 4 according to typecheck.hpp 20, but what about object type ???
      (*currentVariableTable)[currentParameter->identifier->name] = varInfo;
    }
  }

  methodInfo.variables = currentVariableTable;
  methodInfo.localsSize = -1*currentLocalOffset-4; // should this include parameter size?
  methodInfo.parameters = parameters;
  
  node->methodbody->accept(this);

  if(NULL == node->methodbody->returnstatement)
  {
    if(bt_none != methodInfo.returnType.baseType)
    {
      typeError(return_type_mismatch);
    }
  }
  else
  {
    if(methodInfo.returnType.baseType != node->methodbody->returnstatement->basetype)
    {
      typeError(return_type_mismatch);
    }
    if(bt_object == methodInfo.returnType.baseType && methodInfo.returnType.objectClassName != node->methodbody->returnstatement->objectClassName)
    {
      typeError(return_type_mismatch);
    }
  }
  
  (*currentMethodTable)[node->identifier->name] = methodInfo;
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  if (node->statement_list) {
    for(std::list<StatementNode*>::iterator iter = node->statement_list->begin(); iter != node->statement_list->end(); iter++) {
      (*iter)->accept(this);
    }
  }
  if (node->returnstatement) {
    node->returnstatement->accept(this);
  }
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  node->type->accept(this);
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  node->type->accept(this);
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  node->expression->accept(this);
  node->basetype = node->expression->basetype;
  if(bt_object == node->basetype)
  {
    node->objectClassName = node->expression->objectClassName;
  }
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  node->expression->accept(this);

  VariableInfo var1;
  bool foundVar1 = false;
  if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier_1->name))
  {
    std::string searchClassName = currentClassName;
    while("" != searchClassName)
    {
      VariableTable* searchClassMembers = (*classTable)[searchClassName].members;
      if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_1->name)))
      {
        searchClassName = (*classTable)[searchClassName].superClassName;
      }
      else
      {
        var1 = (*searchClassMembers)[node->identifier_1->name];
        foundVar1 = true;
        break;
      }
    }
    if(!foundVar1)
    {
      typeError(undefined_variable);
    }
  }
  else
  {
    var1 = (*currentVariableTable)[node->identifier_1->name];
  }

  if(NULL == node->identifier_2)
  {
    if(var1.type.baseType != node->expression->basetype)
    {
      typeError(assignment_type_mismatch);
    }
    if(bt_object == var1.type.baseType && var1.type.objectClassName != node->expression->objectClassName)
    {
      typeError(assignment_type_mismatch);
    }
  }
  else
  {
    if(bt_object != var1.type.baseType)
    {
      typeError(not_object);
    }

    VariableInfo var2;
    bool foundVar2 = false;
    std::string searchClassName = var1.type.objectClassName;
    while("" != searchClassName)
    {
      VariableTable* searchClassMembers = (*classTable)[searchClassName].members;
      if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_2->name)))
      {
        searchClassName = (*classTable)[searchClassName].superClassName;
      }
      else
      {
        var2 = (*searchClassMembers)[node->identifier_2->name];
        foundVar2 = true;
        break;
      }
    }
    if(!foundVar2)
    {
      typeError(undefined_member);
    }

    if(var2.type.baseType != node->expression->basetype)
    {
      typeError(assignment_type_mismatch);
    }
    if(bt_object == var2.type.baseType && var2.type.objectClassName != node->expression->objectClassName)
    {
      typeError(assignment_type_mismatch);
    }
  }
}

void TypeCheck::visitCallNode(CallNode* node) {
  node->methodcall->accept(this);
}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  node->expression->accept(this);
  if(bt_boolean != node->expression->basetype)
  {
    typeError(if_predicate_type_mismatch);
  }

  if (node->statement_list_1) {
    for(std::list<StatementNode*>::iterator iter = node->statement_list_1->begin(); iter != node->statement_list_1->end(); iter++) {
      (*iter)->accept(this);
    }
  }
  if (node->statement_list_2) {
    for(std::list<StatementNode*>::iterator iter = node->statement_list_2->begin(); iter != node->statement_list_2->end(); iter++) {
      (*iter)->accept(this);
    }
  }
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  node->expression->accept(this);
  if(bt_boolean != node->expression->basetype)
  {
    typeError(while_predicate_type_mismatch);
  }

  if (node->statement_list) {
    for(std::list<StatementNode*>::iterator iter = node->statement_list->begin(); iter != node->statement_list->end(); iter++) {
      (*iter)->accept(this);
    }
  }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  if (node->statement_list) {
    for(std::list<StatementNode*>::iterator iter = node->statement_list->begin(); iter != node->statement_list->end(); iter++) {
      (*iter)->accept(this);
    }
  }
  node->expression->accept(this);
  if(bt_boolean != node->expression->basetype)
  {
    typeError(do_while_predicate_type_mismatch);
  }
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  node->expression->accept(this);
}

void TypeCheck::visitQMNode(QMNode* node) {
  node->expression_1->accept(this);
  if(bt_boolean != node->expression_1->basetype)
  {
    typeError(expression_type_mismatch);
  }
  node->expression_2->accept(this);
  node->expression_3->accept(this);
  if(bt_integer == node->expression_2->basetype && bt_integer == node->expression_3->basetype)
  {
    node->basetype = bt_integer;
  }
  else if(bt_boolean == node->expression_2->basetype && bt_boolean == node->expression_3->basetype)
  {
    node->basetype = bt_boolean;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_integer == node->expression_1->basetype && bt_integer == node->expression_2->basetype)
  {
    node->basetype = bt_integer;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_integer == node->expression_1->basetype && bt_integer == node->expression_2->basetype)
  {
    node->basetype = bt_integer;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_integer == node->expression_1->basetype && bt_integer == node->expression_2->basetype)
  {
    node->basetype = bt_integer;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_integer == node->expression_1->basetype && bt_integer == node->expression_2->basetype)
  {
    node->basetype = bt_integer;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_integer == node->expression_1->basetype && bt_integer == node->expression_2->basetype)
  {
    node->basetype = bt_boolean;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_integer == node->expression_1->basetype && bt_integer == node->expression_2->basetype)
  {
    node->basetype = bt_boolean;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitEqualNode(EqualNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if((bt_integer == node->expression_1->basetype && bt_integer == node->expression_2->basetype) ||
    (bt_boolean == node->expression_1->basetype && bt_boolean == node->expression_2->basetype))
  {
    node->basetype = bt_boolean;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitAndNode(AndNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_boolean == node->expression_1->basetype && bt_boolean == node->expression_2->basetype)
  {
    node->basetype = bt_boolean;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitOrNode(OrNode* node) {
  node->expression_1->accept(this);
  node->expression_2->accept(this);
  if(bt_boolean == node->expression_1->basetype && bt_boolean == node->expression_2->basetype)
  {
    node->basetype = bt_boolean;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitNotNode(NotNode* node) {
  node->expression->accept(this);
  if((bt_boolean == node->expression->basetype))
  {
    node->basetype = bt_boolean;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitNegationNode(NegationNode* node) {
  node->expression->accept(this);
  if((bt_integer == node->expression->basetype))
  {
    node->basetype = bt_integer;
  }
  else
  {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  MethodInfo callMethod;
  if(NULL == node->identifier_2)
  {
    if((*currentMethodTable).end() == (*currentMethodTable).find(node->identifier_1->name))
    {
      bool foundMethod = false;
      std::string searchClassName = (*classTable)[currentClassName].superClassName;
      while("" != searchClassName)
      {
        MethodTable* searchClassMethods = (*classTable)[searchClassName].methods;
        if((*searchClassMethods).end() == ((*searchClassMethods).find(node->identifier_1->name)))
        {
          searchClassName = (*classTable)[searchClassName].superClassName;
        }
        else
        {
          callMethod = (*searchClassMethods)[node->identifier_1->name];
          foundMethod = true;
          break;
        }
      }
      if(!foundMethod)
      {
        typeError(undefined_method);
      }
    }
    else
    {
      callMethod = (*currentMethodTable)[node->identifier_1->name];
    }
  }
  else
  {
    VariableInfo var1;
    bool foundVar1 = false;
    if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier_1->name))
    {
      std::string searchClassName = currentClassName;
      while("" != searchClassName)
      {
        VariableTable* searchClassMembers = (*classTable)[searchClassName].members;
        if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_1->name)))
        {
          searchClassName = (*classTable)[searchClassName].superClassName;
        }
        else
        {
          var1 = (*searchClassMembers)[node->identifier_1->name];
          foundVar1 = true;
          break;
        }
      }
      if(!foundVar1)
      {
        typeError(undefined_variable);
      }
    }
    else
    {
      var1 = (*currentVariableTable)[node->identifier_1->name];
    }

    if(bt_object != var1.type.baseType)
    {
      typeError(not_object);
    }
    bool foundMethod = false;
    std::string searchClassName = var1.type.objectClassName;
    while("" != searchClassName)
    {
      MethodTable* searchClassMethods = (*classTable)[searchClassName].methods;
      if((*searchClassMethods).end() == ((*searchClassMethods).find(node->identifier_2->name)))
      {
        searchClassName = (*classTable)[searchClassName].superClassName;
      }
      else
      {
        callMethod = (*searchClassMethods)[node->identifier_2->name];
        foundMethod = true;
        break;
      }
    }
    if(!foundMethod)
    {
      typeError(undefined_method);
    }
  }

  if((*callMethod.parameters).size() !=  node->expression_list->size())
  {
    typeError(argument_number_mismatch);
  }
  if (node->expression_list) {
    for(std::list<ExpressionNode*>::iterator iter = node->expression_list->begin(); iter != node->expression_list->end(); iter++) {
      (*iter)->accept(this);
    }
  }
  std::list<CompoundType>::iterator iter1 = (*callMethod.parameters).begin();
  std::list<ExpressionNode*>::iterator iter2 = node->expression_list->begin();
  while(iter1 != (*callMethod.parameters).end())
  {
    if((*iter1).baseType != (*iter2)->basetype)
    {
      typeError(argument_type_mismatch);
    }
    if(bt_object == (*iter1).baseType && (*iter1).objectClassName != (*iter2)->objectClassName)
    {
      typeError(argument_type_mismatch);
    }
    iter1++;
    iter2++;
  }
  
  node->basetype = callMethod.returnType.baseType;
  if(bt_object == node->basetype)
  {
    node->objectClassName = callMethod.returnType.objectClassName;
  }
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  VariableInfo var1;
  bool foundVar1 = false;
  if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier_1->name))
  {
    std::string searchClassName = currentClassName;
    while("" != searchClassName)
    {
      VariableTable* searchClassMembers = (*classTable)[searchClassName].members;
      if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_1->name)))
      {
        searchClassName = (*classTable)[searchClassName].superClassName;
      }
      else
      {
        var1 = (*searchClassMembers)[node->identifier_1->name];
        foundVar1 = true;
        break;
      }
    }
    if(!foundVar1)
    {
      typeError(undefined_variable);
    }
  }
  else
  {
    var1 = (*currentVariableTable)[node->identifier_1->name];
  }

  if(bt_object != var1.type.baseType)
  {
    typeError(not_object);
  }

  VariableInfo var2;
  bool foundVar2 = false;
  std::string searchClassName = var1.type.objectClassName;
  while("" != searchClassName)
  {
    VariableTable* searchClassMembers = (*classTable)[searchClassName].members;
    if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_2->name)))
    {
      searchClassName = (*classTable)[searchClassName].superClassName;
    }
    else
    {
      var2 = (*searchClassMembers)[node->identifier_2->name];
      foundVar2 = true;
      break;
    }
  }
  if(!foundVar2)
  {
    typeError(undefined_member);
  }

  node->basetype = var2.type.baseType;
  if(bt_object == node->basetype)
  {
    node->objectClassName = var2.type.objectClassName;
  }
}

void TypeCheck::visitVariableNode(VariableNode* node) {
  VariableInfo var1;
  bool foundVar1 = false;
  if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier->name))
  {
    std::string searchClassName = currentClassName;
    while("" != searchClassName)
    {
      VariableTable* searchClassMembers = (*classTable)[searchClassName].members;
      if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier->name)))
      {
        searchClassName = (*classTable)[searchClassName].superClassName;
      }
      else
      {
        var1 = (*searchClassMembers)[node->identifier->name];
        foundVar1 = true;
        break;
      }
    }
    if(!foundVar1)
    {
      typeError(undefined_variable);
    }
  }
  else
  {
    var1 = (*currentVariableTable)[node->identifier->name];
  }

  node->basetype = var1.type.baseType;
  if(bt_object == node->basetype)
  {
    node->objectClassName = var1.type.objectClassName;
  }
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  node->basetype = bt_integer;
  //node->integer->accept(this);
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  node->basetype = bt_boolean;
  //node->integer->accept(this);
}

void TypeCheck::visitNewNode(NewNode* node) {
  if((*classTable).end() == (*classTable).find(node->identifier->name))
  {
    typeError(undefined_class);
  }

  MethodTable* classMethods = (*classTable)[node->identifier->name].methods;

  MethodInfo constructor;
  if((*classMethods).end() == (*classMethods).find(node->identifier->name))
  {
    // typeError(undefined_method);
    constructor.returnType.baseType = bt_none;
    constructor.variables = new VariableTable();
    constructor.parameters = new std::list<CompoundType>();
    constructor.localsSize = 0;
  }
  else
  {
    constructor = (*classMethods)[node->identifier->name];
  }

  
  if(NULL != node->expression_list)
  {
    if((*constructor.parameters).size() !=  node->expression_list->size())
    {
      typeError(argument_number_mismatch);
    }
    if (node->expression_list) {
      for(std::list<ExpressionNode*>::iterator iter = node->expression_list->begin(); iter != node->expression_list->end(); iter++) {
        (*iter)->accept(this);
      }
    }
    std::list<CompoundType>::iterator iter1 = (*constructor.parameters).begin();
    std::list<ExpressionNode*>::iterator iter2 = node->expression_list->begin();
    while(iter1 != (*constructor.parameters).end())
    {
      if((*iter1).baseType != (*iter2)->basetype)
      {
        typeError(argument_type_mismatch);
      }
      if(bt_object == (*iter1).baseType && (*iter1).objectClassName != (*iter2)->objectClassName)
      {
        typeError(argument_type_mismatch);
      }
      iter1++;
      iter2++;
    }
  }
  else
  {
    if(0 != (*constructor.parameters).size())
    {
      typeError(argument_number_mismatch);
    }
  }
  

  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  node->basetype = bt_boolean;
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
  if(currentClassName == node->objectClassName || (*classTable).end() == (*classTable).find(node->objectClassName))
  {
    typeError(undefined_class);
  }
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  node->basetype = bt_none;
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
  // WRITEME: Replace with code if necessary
}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
