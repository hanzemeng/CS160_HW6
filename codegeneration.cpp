#include "codegeneration.hpp"

// CodeGenerator Visitor Functions: These are the functions
// you will complete to generate the x86 assembly code. Not
// all functions must have code, many may be left empty.

void CodeGenerator::visitProgramNode(ProgramNode* node) {
    std::cout << ".globl Main_main" << std::endl;
    std::cout << std::endl;

    std::cout << ".data" << std::endl;
    std::cout << "printstr: .asciz \"%d\\n\"" << std::endl;
    std::cout << std::endl;

    std::cout << ".text" << std::endl;
    node->visit_children(this);
}

void CodeGenerator::visitClassNode(ClassNode* node) {
    currentClassName = node->identifier_1->name;
    if (node->method_list) {
        for(std::list<MethodNode*>::iterator iter = node->method_list->begin(); iter != node->method_list->end(); iter++) {
        (*iter)->accept(this);
        }
    }
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
    currentMethodName = node->identifier->name;
    std::cout << currentClassName << "_" << currentMethodName << ":" << std::endl;
    std::cout << "push %ebp" << std::endl;
    std::cout << "mov %esp, %ebp" << std::endl;
    std::cout << "sub $" << (*(*classTable)[currentClassName].methods)[currentMethodName].localsSize << ", %esp" << std::endl;

    node->methodbody->accept(this);

    std::cout << "mov %ebp, %esp" << std::endl;
    std::cout << "pop %ebp" << std::endl;
    std::cout << "ret" << std::endl;
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
    if (node->statement_list) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list->begin(); iter != node->statement_list->end(); iter++) {
        (*iter)->accept(this);
        }
    }
    if (node->returnstatement) {
        node->returnstatement->accept(this);
    }
}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
    node->expression->accept(this);
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
    VariableInfo assignedVar;
    node->expression->accept(this);
    if(NULL == node->identifier_2)
    {
        VariableTable* currentVariableTable = (*(*classTable)[currentClassName].methods)[currentMethodName].variables;
        if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier_1->name))
        {
            int varOffset = 0;
            std::string searchClassName = currentClassName;
            std::stack<VariableTable*> variableTables;
            std::stack<int> variableTablesSizes;
            while("" != searchClassName)
            {
                variableTables.push((*classTable)[searchClassName].members);
                variableTablesSizes.push((*classTable)[searchClassName].membersSize);
                searchClassName = (*classTable)[searchClassName].superClassName;
            }

            while(!variableTables.empty())
            {
                VariableTable* searchClassMembers = variableTables.top();
                int searchClassMembersSize = variableTablesSizes.top();
                variableTables.pop();
                variableTablesSizes.pop();
                if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_1->name)))
                {
                    varOffset += searchClassMembersSize;
                }
                else
                {
                    assignedVar = (*searchClassMembers)[node->identifier_1->name];
                    varOffset += assignedVar.offset;
                    std::cout << "mov 8(%ebp), %ebx" << std::endl;
                    std::cout << "mov %eax, " << varOffset << "(%ebx)" << std::endl;
                    break;
                }
            }
        }
        else
        {
            assignedVar = (*currentVariableTable)[node->identifier_1->name];
            std::cout << "mov %eax, " << assignedVar.offset << "(%ebp)" << std::endl;
        }
    }
    else
    {
        VariableInfo firstVar;
        if((*(*(*classTable)[currentClassName].methods)[currentMethodName].variables).end() == 
        (*(*(*classTable)[currentClassName].methods)[currentMethodName].variables).find(node->identifier_1->name))
        {
            int firstVarOffset = 0;
            std::string searchClassName = currentClassName;
            std::stack<VariableTable*> variableTables;
            std::stack<int> variableTablesSizes;
            while("" != searchClassName)
            {
                variableTables.push((*classTable)[searchClassName].members);
                variableTablesSizes.push((*classTable)[searchClassName].membersSize);
                searchClassName = (*classTable)[searchClassName].superClassName;
            }

            while(!variableTables.empty())
            {
                VariableTable* searchClassMembers = variableTables.top();
                int searchClassMembersSize = variableTablesSizes.top();
                variableTables.pop();
                variableTablesSizes.pop();
                if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_1->name)))
                {
                    firstVarOffset += searchClassMembersSize;
                }
                else
                {
                    assignedVar = (*searchClassMembers)[node->identifier_1->name];
                    firstVarOffset += assignedVar.offset;
                    std::cout << "mov 8(%ebp), %ebx" << std::endl;
                    std::cout << "mov " << firstVarOffset << "(%ebx), %ebx" << std::endl;
                    break;
                }
            }
        }
        else
        {
            firstVar = (*(*(*classTable)[currentClassName].methods)[currentMethodName].variables)[node->identifier_1->name];
            std::cout << "mov " << firstVar.offset << "(%ebp), %ebx" << std::endl;
        }

        int secondVarOffset = 0;
        std::string searchClassName = firstVar.type.objectClassName;
        std::stack<VariableTable*> variableTables;
        std::stack<int> variableTablesSizes;
        while("" != searchClassName)
        {
            variableTables.push((*classTable)[searchClassName].members);
            variableTablesSizes.push((*classTable)[searchClassName].membersSize);
            searchClassName = (*classTable)[searchClassName].superClassName;
        }

        while(!variableTables.empty())
        {
            VariableTable* searchClassMembers = variableTables.top();
            int searchClassMembersSize = variableTablesSizes.top();
            variableTables.pop();
            variableTablesSizes.pop();
            if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_2->name)))
            {
                secondVarOffset += searchClassMembersSize;
            }
            else
            {
                assignedVar = (*searchClassMembers)[node->identifier_2->name];
                secondVarOffset += assignedVar.offset;
                break;
            }
        }
        std::cout << "mov %eax, " << secondVarOffset << "(%ebx)" << std::endl;
    }
}

void CodeGenerator::visitCallNode(CallNode* node) {
    node->methodcall->accept(this);
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
    int firstLabel = currentLabel;
    nextLabel();
    int secondLabel = currentLabel;
    nextLabel();

    node->expression->accept(this);
    std::cout << "mov $0, %edx" << std::endl;
    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "je " << "L" << firstLabel << std::endl;

    if (node->statement_list_1) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list_1->begin(); iter != node->statement_list_1->end(); iter++) {
        (*iter)->accept(this);
        }
    }
    std::cout << "jmp " << "L" << secondLabel << std::endl;

    std::cout << "L" << firstLabel << ":" << std::endl;
    if (node->statement_list_2) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list_2->begin(); iter != node->statement_list_2->end(); iter++) {
        (*iter)->accept(this);
        }
    }
    std::cout << "L" << secondLabel << ":" << std::endl;
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
    int firstLabel = currentLabel;
    nextLabel();
    int secondLabel = currentLabel;
    nextLabel();

    std::cout << "L" << firstLabel << ":" << std::endl;
    node->expression->accept(this);
    std::cout << "mov $0, %edx" << std::endl;
    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "je " << "L" << secondLabel << std::endl;

    if (node->statement_list) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list->begin(); iter != node->statement_list->end(); iter++) {
        (*iter)->accept(this);
        }
    }

    std::cout << "jmp " << "L" << firstLabel << std::endl;
    std::cout << "L" << secondLabel << ":" << std::endl;
}

void CodeGenerator::visitPrintNode(PrintNode* node) {
    node->expression->accept(this);
    std::cout << "push %eax" << std::endl;
    std::cout << "push $printstr" << std::endl;
    std::cout << "call printf" << std::endl;
    std::cout << "add $8, %esp" << std::endl;
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
    int firstLabel = currentLabel;
    nextLabel();
    int secondLabel = currentLabel;
    nextLabel();

    std::cout << "L" << firstLabel << ":" << std::endl;
    if (node->statement_list) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list->begin(); iter != node->statement_list->end(); iter++) {
        (*iter)->accept(this);
        }
    }

    node->expression->accept(this);
    std::cout << "mov $0, %edx" << std::endl;
    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "je " << "L" << secondLabel << std::endl;
    std::cout << "jmp " << "L" << firstLabel << std::endl;
    std::cout << "L" << secondLabel << ":" << std::endl;
}

void CodeGenerator::visitQMNode(QMNode* node){
    int firstLabel = currentLabel;
    nextLabel();
    int secondLabel = currentLabel;
    nextLabel();

    node->expression_1->accept(this);
    std::cout << "mov $0, %edx" << std::endl;
    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "je " << "L" << firstLabel << std::endl;

    node->expression_2->accept(this);
    std::cout << "jmp " << "L" << secondLabel << std::endl;

    std::cout << "L" << firstLabel << ":" << std::endl;
    node->expression_3->accept(this);

    std::cout << "L" << secondLabel << ":" << std::endl;
}

void CodeGenerator::visitPlusNode(PlusNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "add %edx, %eax" << std::endl;
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "sub %edx, %eax" << std::endl;
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "imul %edx, %eax" << std::endl;
}

void CodeGenerator::visitDivideNode(DivideNode* node) { // cause problem
    /*
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "idiv %edx, %eax" << std::endl;
    */
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    //std::cout << "mov %eax, %ecx" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %ecx" << std::endl;
    std::cout << "cdq" << std::endl;
    std::cout << "idiv %ecx" << std::endl;
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;

    int firstLabel = currentLabel;
    nextLabel();
    int secondLabel = currentLabel;
    nextLabel();

    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "jg " << "L" << firstLabel << std::endl;
    std::cout << "mov $0, %eax" << std::endl;
    std::cout << "jmp " << "L" << secondLabel << std::endl;
    std::cout << "L" << firstLabel << ":" << std::endl;
    std::cout << "mov $1, %eax" << std::endl;
    std::cout << "L" << secondLabel << ":" << std::endl;
}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;

    int firstLabel = currentLabel;
    nextLabel();
    int secondLabel = currentLabel;
    nextLabel();

    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "jge " << "L" << firstLabel << std::endl;
    std::cout << "mov $0, %eax" << std::endl;
    std::cout << "jmp " << "L" << secondLabel << std::endl;
    std::cout << "L" << firstLabel << ":" << std::endl;
    std::cout << "mov $1, %eax" << std::endl;
    std::cout << "L" << secondLabel << ":" << std::endl;
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;

    int firstLabel = currentLabel;
    nextLabel();
    int secondLabel = currentLabel;
    nextLabel();

    std::cout << "cmp %edx, %eax" << std::endl;
    std::cout << "je " << "L" << firstLabel << std::endl;
    std::cout << "mov $0, %eax" << std::endl;
    std::cout << "jmp " << "L" << secondLabel << std::endl;
    std::cout << "L" << firstLabel << ":" << std::endl;
    std::cout << "mov $1, %eax" << std::endl;
    std::cout << "L" << secondLabel << ":" << std::endl;
}

void CodeGenerator::visitAndNode(AndNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "and %edx, %eax" << std::endl;
}

void CodeGenerator::visitOrNode(OrNode* node) {
    node->expression_2->accept(this);
    std::cout << "push %eax" << std::endl;
    node->expression_1->accept(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "or %edx, %eax" << std::endl;
}

void CodeGenerator::visitNotNode(NotNode* node) {
    node->expression->accept(this);
    std::cout << "xor $1, %eax" << std::endl;
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
    node->expression->accept(this);
    std::cout << "imul $-1, %eax" << std::endl;
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
    int offsetSize = 4;
    if (node->expression_list) {
        for(std::list<ExpressionNode*>::reverse_iterator iter = node->expression_list->rbegin();iter != node->expression_list->rend(); iter++) {
            (*iter)->accept(this);
            std::cout << "push %eax" << std::endl;
            offsetSize += 4;
        }
    }
    
    if(NULL == node->identifier_2)
    {
        std::cout << "push 8(%ebp)" << std::endl;
        std::string searchClassName = currentClassName;
        while("" != searchClassName)
        {
            MethodTable* searchClassMethods = (*classTable)[searchClassName].methods;
            if((*searchClassMethods).end() == ((*searchClassMethods).find(node->identifier_1->name)))
            {
                searchClassName = (*classTable)[searchClassName].superClassName;
            }
            else
            {
                std::cout << "call " << searchClassName << "_" << node->identifier_1->name << std::endl;
                break;
            }
        }
    }
    else
    {
        VariableInfo var1;
        VariableTable* currentVariableTable = (*(*classTable)[currentClassName].methods)[currentMethodName].variables;
        if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier_1->name))
        {
            int var1Offset = 0;
            std::string searchClassName = currentClassName;
            std::stack<VariableTable*> variableTables;
            std::stack<int> variableTablesSizes;
            while("" != searchClassName)
            {
                variableTables.push((*classTable)[searchClassName].members);
                variableTablesSizes.push((*classTable)[searchClassName].membersSize);
                searchClassName = (*classTable)[searchClassName].superClassName;
            }

            while(!variableTables.empty())
            {
                VariableTable* searchClassMembers = variableTables.top();
                int searchClassMembersSize = variableTablesSizes.top();
                variableTables.pop();
                variableTablesSizes.pop();
                if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_1->name)))
                {
                    var1Offset += searchClassMembersSize;
                }
                else
                {
                    var1 = (*searchClassMembers)[node->identifier_1->name];
                    var1Offset += var1.offset;
                    std::cout << "push " << var1Offset << "(%ebp)" << std::endl;
                    break;
                }
            }
        }
        else
        {
            var1 = (*currentVariableTable)[node->identifier_1->name];
            std::cout << "push " << var1.offset << "(%ebp)" << std::endl;
        }

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
                
                break;
            }
        }
        std::cout << "call " << searchClassName << "_" << node->identifier_2->name << std::endl;
        
    }
    std::cout << "add $" << offsetSize << ", %esp" << std::endl;
}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
    VariableInfo var1;
    VariableTable* currentVariableTable = (*(*classTable)[currentClassName].methods)[currentMethodName].variables;
    if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier_1->name))
    {
        int firstVarOffset = 0;
        std::string searchClassName = currentClassName;
        std::stack<VariableTable*> variableTables;
        std::stack<int> variableTablesSizes;
        while("" != searchClassName)
        {
            variableTables.push((*classTable)[searchClassName].members);
            variableTablesSizes.push((*classTable)[searchClassName].membersSize);
            searchClassName = (*classTable)[searchClassName].superClassName;
        }

        while(!variableTables.empty())
        {
            VariableTable* searchClassMembers = variableTables.top();
            int searchClassMembersSize = variableTablesSizes.top();
            variableTables.pop();
            variableTablesSizes.pop();
            if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_1->name)))
            {
                firstVarOffset += searchClassMembersSize;
            }
            else
            {
                var1 = (*searchClassMembers)[node->identifier_1->name];
                firstVarOffset += var1.offset;
                std::cout << "mov 8(%ebp), %ebx" << std::endl;
                std::cout << "mov " << firstVarOffset << "(%ebx), %ebx" << std::endl;
                break;
            }
        }
    }
    else
    {
        var1 = (*currentVariableTable)[node->identifier_1->name];
        std::cout << "mov " << var1.offset << "(%ebp), %ebx" << std::endl;
    }

    VariableInfo var2;
    int secondVarOffset = 0;
    std::string searchClassName = var1.type.objectClassName;
    std::stack<VariableTable*> variableTables;
    std::stack<int> variableTablesSizes;
    while("" != searchClassName)
    {
        variableTables.push((*classTable)[searchClassName].members);
        variableTablesSizes.push((*classTable)[searchClassName].membersSize);
        searchClassName = (*classTable)[searchClassName].superClassName;
    }

    while(!variableTables.empty())
    {
        VariableTable* searchClassMembers = variableTables.top();
        int searchClassMembersSize = variableTablesSizes.top();
        variableTables.pop();
        variableTablesSizes.pop();
        if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier_2->name)))
        {
            secondVarOffset += searchClassMembersSize;
        }
        else
        {
            var2 = (*searchClassMembers)[node->identifier_2->name];
            secondVarOffset += var2.offset;
            std::cout << "mov " << secondVarOffset << "(%ebx), %eax" << std::endl;
            break;
        }
    }
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
    VariableInfo var1;
    VariableTable* currentVariableTable = (*(*classTable)[currentClassName].methods)[currentMethodName].variables;
    if((*currentVariableTable).end() == (*currentVariableTable).find(node->identifier->name))
    {
        int firstVarOffset = 0;
        std::string searchClassName = currentClassName;
        std::stack<VariableTable*> variableTables;
        std::stack<int> variableTablesSizes;
        while("" != searchClassName)
        {
            variableTables.push((*classTable)[searchClassName].members);
            variableTablesSizes.push((*classTable)[searchClassName].membersSize);
            searchClassName = (*classTable)[searchClassName].superClassName;
        }

        while(!variableTables.empty())
        {
            VariableTable* searchClassMembers = variableTables.top();
            int searchClassMembersSize = variableTablesSizes.top();
            variableTables.pop();
            variableTablesSizes.pop();
            if((*searchClassMembers).end() == ((*searchClassMembers).find(node->identifier->name)))
            {
                firstVarOffset += searchClassMembersSize;
            }
            else
            {
                var1 = (*searchClassMembers)[node->identifier->name];
                firstVarOffset += var1.offset;
                std::cout << "mov 8(%ebp), %ebx" << std::endl;
                std::cout << "mov " << firstVarOffset << "(%ebx), %eax" << std::endl;
                break;
            }
        }
    }
    else
    {
        var1 = (*currentVariableTable)[node->identifier->name];
        std::cout << "mov " << var1.offset << "(%ebp), %eax" << std::endl;
    }
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    std::cout << "mov $" << node->integer->value << ", %eax" <<std::endl;
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    std::cout << "mov $" << node->integer->value << ", %eax" <<std::endl;
}

void CodeGenerator::visitNewNode(NewNode* node) {
    int allocateSize = 0;
    std::string searchClassName = node->identifier->name;
    while("" != searchClassName)
    {
        allocateSize += (*classTable)[searchClassName].membersSize;
        searchClassName = (*classTable)[searchClassName].superClassName;
    }

    int offsetSize = 0;
    if(NULL != node->expression_list)
    {
        for(std::list<ExpressionNode*>::reverse_iterator iter = node->expression_list->rbegin();iter != node->expression_list->rend(); iter++) {
            (*iter)->accept(this);
            std::cout << "push %eax" << std::endl;
            offsetSize += 4;
        }
    }

    std::cout << "push $" << allocateSize << std::endl;
    std::cout << "call malloc" << std::endl;
    std::cout << "add $4, %esp" << std::endl;

    if(NULL != node->expression_list)
    {
        std::cout << "push %eax" << std::endl;
        std::cout << "call " << node->identifier->name << "_" << node->identifier->name << std::endl;
        std::cout << "pop %eax" << std::endl;
        std::cout << "add $" << offsetSize << ", %esp" << std::endl;
    }
}

void CodeGenerator::visitIntegerTypeNode(IntegerTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitBooleanTypeNode(BooleanTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitObjectTypeNode(ObjectTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitNoneNode(NoneNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIdentifierNode(IdentifierNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIntegerNode(IntegerNode* node) {
    // WRITEME: Replace with code if necessary
}