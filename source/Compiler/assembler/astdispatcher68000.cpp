#include "astdispatcher68000.h"


ASTDispather68000::ASTDispather68000()
{

}

void ASTDispather68000::dispatch(NodeBinOP *node)
{
    node->DispatchConstructor();
    // First check if both are consants:

    // First, flip such that anything numeric / pure var is to the right
/*    if (node->m_op.m_type == TokenType::MUL || node->m_op.m_type == TokenType::PLUS)
    if (!(node->m_right->isPureNumeric() || node->m_right->isPureVariable())) {
        if (node->m_left->isPureNumeric() || node->m_left->isPureVariable()) {
            Node* tmp = node->m_right;
            node->m_right = node->m_left;
            node->m_left = tmp;
        }
    }
*/

    if (node->isPureNumeric()) {
        //qDebug() << "IS PURE NUMERIC BinOp";
        int val = node->BothPureNumbersBinOp(as);
        QString s = "";
        if (node->m_left->isAddress() || node->m_right->isAddress())
            s = "";
        //qDebug() << "A0";
        QString d0 = as->m_regAcc.Get();
        //qDebug() << "A05";
        as->Asm("move "+s+node->getValue() +","+d0);
        //qDebug() << "A07";
        as->m_regAcc.Pop(d0);
        as->m_varStack.push(d0);
        //qDebug() << "A1";
//        StoreVariable(as,)
        return;
    }

//    qDebug() << "Varstack count: " << as->m_varStack.m_vars.count();
  //  qDebug() << "Binop: " << as->m_varStack.m_vars.count();
    as->BinOP(node->m_op.m_type);
    QString op = as->m_varStack.pop();


/*    Node* tmp = node->m_left;
    node->m_left = node->m_right;
    node->m_right = tmp;
*/

    /*
      // WORKS
    QString d0 = as->m_regAcc.Get();
    as->m_varStack.push(d0);

    node->m_right->Accept(this);
    QString right = as->m_varStack.pop();
    node->m_left->Accept(this);

    TransformVariable(as,"move.l",d0,as->m_varStack.pop());
    TransformVariable(as,op+".l",d0,right);
*/


//    qDebug() << "Start BINOP";
    QString d0 = "";
    bool start = false;
    if (node->m_left->isPureNumeric() || node->m_left->isPureVariable())
        start = true;


    if (start) {
        QString l = as->m_regAcc.m_latest;
        d0 = as->m_regAcc.Get();
//        as->Comment("NEW register: " + d0);
        as->m_regAcc.m_latest = d0;
    }
    else {
        node->m_left->Accept(this);
        d0 = as->m_regAcc.m_latest;
  //      as->Comment("Peeklatest : " + d0);

    }

    if (start) {
//        NodeVar nv =
        node->m_left->Accept(this);
  //      LoadVariable(node->m_left);
    //    as->Comment("Start : " + d0);

        TransformVariable(as,"move",d0,as->m_varStack.pop(),node->m_left);
    }

    node->m_right->Accept(this);
    QString right = as->m_varStack.pop();

    if (op.toLower().contains("mul") || op.toLower().contains("div"))
        op = op+".w"; else op=op +m_lastSize;//+".l";

//    as->Comment("d0 used:" +d0);

    TransformVariable(as,op,d0,right);
//    as->Comment("operation done  :  "+op+" : "  + d0 + "  varstack : " + as->m_varStack.current() + "   cnt: " +QString::number(as->m_varStack.m_vars.count()));
  //  as->Comment("operation done  :  "+op+" : "  + d0 + "  regstack : " + as->m_regAcc.m_latest + "   cnt: " +QString::number(as->m_regAcc.m_free.count()));
 //   if (as->m_varStack.m_vars.count()==1) // EOL {
   //     as->m_varStack.m_vars[0] = d0; // Replace with current for assignment
 //   as->Comment("POP register: " + d0);
    as->m_regAcc.Pop(d0);
    as->m_varStack.push(d0);
    as->m_regAcc.m_latest =d0;

}

void ASTDispather68000::dispatch(NodeNumber *node)
{
    as->m_varStack.push(node->getValue());

}

void ASTDispather68000::dispatch(NodeAsm *node)
{
    node->DispatchConstructor();

    QStringList txt = node->m_asm.split("\n");
    as->Comment("");
    as->Comment("****** Inline assembler section");
    for (QString t: txt) {
        as->Write(t,0);
    }
    as->Asm("");

}

void ASTDispather68000::dispatch(NodeString *node)
{

}

void ASTDispather68000::dispatch(NodeUnaryOp *node)
{
  //  node->DispatchConstructor();
//    node->Accept(this);
//    if (node->isMinusOne())
    if (!(node->m_op.m_type==TokenType::MINUS))
        node->m_right->Accept(this);
    else {
        NodeNumber*n = dynamic_cast<NodeNumber*>(node->m_right);
        n->m_val*=-1;
        node->m_right->Accept(this);
    }

}

void ASTDispather68000::dispatch(NodeCompound *node)
{
    node->DispatchConstructor();

    as->BeginBlock();
    for (Node* n: node->children)
        n->Accept(this);


    as->EndBlock();

}

void ASTDispather68000::dispatch(NodeVarDecl *node)
{
    node->DispatchConstructor();


    node->ExecuteSym(as->m_symTab);



    NodeVar* v = (NodeVar*)node->m_varNode;
    NodeVarType* t = (NodeVarType*)node->m_typeNode;
    if (t->m_flags.contains("chipmem")) {
        as->m_currentBlock = &as->m_chipMem;
    }

    if (t->m_op.m_type==TokenType::ARRAY) {
        as->DeclareArray(v->value, t->m_arrayVarType.m_value, t->m_op.m_intVal, t->m_data, t->m_position);
        node->m_dataSize=t->m_op.m_intVal;
        as->m_symTab->Lookup(v->value, node->m_op.m_lineNumber)->m_type="address";
        as->m_symTab->Lookup(v->value, node->m_op.m_lineNumber)->m_arrayType=t->m_arrayVarType.m_type;
    }else
    if (t->m_op.m_type==TokenType::STRING) {
        as->DeclareString(v->value, t->m_data);

/*        node->m_dataSize = 0;
        for (QString s: t->m_data)
            node->m_dataSize+=s.count();
        node->m_dataSize++; // 0 end*/
    }
    else
    if (t->m_op.m_type==TokenType::CSTRING) {
        as->DeclareCString(v->value, t->m_data);
        node->m_dataSize = 0;
        for (QString s: t->m_data)
            node->m_dataSize+=s.count();
        node->m_dataSize++; // 0 end
    }
    else
    if (t->m_op.m_type==TokenType::INCBIN) {
        if (node->m_curMemoryBlock!=nullptr)
            ErrorHandler::e.Error("IncBin can not be declared within a user-defined memory block :",node->m_op.m_lineNumber);

        IncBin(as,node);
    }
    else
    if (t->m_op.m_type==TokenType::POINTER) {
        if (node->m_curMemoryBlock!=nullptr)
            ErrorHandler::e.Error("Pointers can not be declared within a user-defined memory block :",node->m_op.m_lineNumber);
//        DeclarePointer(node);
    }else {
        node->m_dataSize=1;
        if (t->value.toLower()=="integer") node->m_dataSize = 2;
        if (t->value.toLower()=="long") node->m_dataSize = 4;
        as->DeclareVariable(v->value, t->value, t->initVal);
    }



    as->m_currentBlock = nullptr;


}

void ASTDispather68000::dispatch(NodeBlock *node)
{

    node->DispatchConstructor();

    as->PushBlock(node->m_currentLineNumber);


    bool blockLabel = false;
    bool blockProcedure = false;
    bool hasLabel = false;

    QString label = as->NewLabel("block");


    if (node->m_decl.count()!=0) {
        as->Asm("jmp " + label);
        hasLabel = true;
        //           as->PushBlock(m_decl[0]->m_op.m_lineNumber-1);
    }
    for (Node* n: node->m_decl) {
        if (dynamic_cast<NodeVarDecl*>(n)==nullptr) {
            if (!blockProcedure) // Print label at end of vardecl
            {
                if (n->m_op.m_lineNumber!=0) {
                    //                      as->PopBlock(n->m_op.m_lineNumber);
                    blockProcedure = true;
                    //   qDebug() << "pop" << n->m_op.m_lineNumber << " " << TokenType::getType(n->getType(as));
                }

            }

        }
        //if (dynamic_cast<NodeProcedureDecl*>(n)==nullptr)
        //qDebug() << "VarDeclBuild:" ;
        n->Accept(this);

    }
    as->VarDeclEnds();
//    if (node->m_decl.count()!=0)
        as->Asm(" 	CNOP 0,4");

    if (!blockLabel && hasLabel) {
        as->Label(label);
    }

    if (node->forceLabel!="")
        as->Label(node->forceLabel);

    if (node->m_compoundStatement!=nullptr)
        node->m_compoundStatement->Accept(this);


    as->PopBlock(node->m_currentLineNumber);


}

void ASTDispather68000::dispatch(NodeProgram *node)
{
    node->DispatchConstructor();

//    as->EndMemoryBlock();
    NodeBuiltinMethod::m_isInitialized.clear();
    as->Program(node->m_name, node->m_param);
  //  as->m_source << node->m_initJumps;
    node->m_NodeBlock->m_isMainBlock = true;
    node->m_NodeBlock->Accept(this);

//    qDebug() << as->m_currentBlock;
    as->EndProgram();

}

void ASTDispather68000::dispatch(NodeVarType *node)
{

}

void ASTDispather68000::dispatch(NodeBinaryClause *node)
{

}

void ASTDispather68000::dispatch(NodeProcedure *node)
{
    node->DispatchConstructor();


    if (node->m_parameters.count()!=node->m_procedure->m_paramDecl.count())
        ErrorHandler::e.Error("Procedure '" + node->m_procedure->m_procName+"' requires "
                              + QString::number(node->m_procedure->m_paramDecl.count()) +" parameters, not "
                              + QString::number(node->m_parameters.count()) + ".", node->m_op.m_lineNumber);

    for (int i=0; i<node->m_parameters.count();i++) {
        // Assign all variables
        NodeVarDecl* vd = (NodeVarDecl*)node->m_procedure->m_paramDecl[i];
        NodeAssign* na = new NodeAssign(vd->m_varNode, node->m_parameters[i]->m_op, node->m_parameters[i]);
        na->Accept(this);
//        na->Build(as);
    }

    as->Asm("jsr " + node->m_procedure->m_procName);

}

void ASTDispather68000::dispatch(NodeProcedureDecl *node)
{
    node->DispatchConstructor();

    bool isInitFunction=false;
    bool isBuiltinFunction=false;
    if (Syntax::s.builtInFunctions.contains(node->m_procName)) {
        isBuiltinFunction = true;
        isInitFunction = Syntax::s.builtInFunctions[node->m_procName].m_initFunction;
    }

    as->Asm("");
    as->Asm("");
    as->Comment("***********  Defining procedure : " + node->m_procName);
    QString type = (isBuiltinFunction) ? "Built-in function" : "User-defined procedure";
    as->Comment("   Procedure type : " + type);
    if (isBuiltinFunction) {
        type = (isInitFunction) ? "yes" : "no";
        as->Comment("   Requires initialization : " + type);
    }
    as->Asm("");


    if (!isInitFunction) {
        //as->Asm("jmp afterProc_" + m_procName);


        //as->Label(m_procName);
    }
//    if (m_isInterrupt)
  //      as->Asm("dec $d019        ; acknowledge IRQ");
    if (node->m_block!=nullptr) {
        NodeBlock* b = dynamic_cast<NodeBlock*>(node->m_block);
        if (b!=nullptr)
            b->forceLabel=node->m_procName;
        node->m_block->Accept(this);
//        node->m_block->Build(as);
    }
    if (!isInitFunction) {
        if (node->m_type==0) {
            as->Asm("rts");
        }
        else as->Asm("rti");
      //as->Label("afterProc_" + m_procName);
    }



}

void ASTDispather68000::dispatch(NodeConditional *node)
{
    QString labelStartOverAgain = as->NewLabel("while");
    QString lblstartTrueBlock = as->NewLabel("ConditionalTrueBlock");

    QString labelElse = as->NewLabel("elseblock");
    QString labelElseDone = as->NewLabel("elsedoneblock");
    // QString labelFailed = as->NewLabel("conditionalfailed");

//    qDebug() << "HMM";

    if (node->m_isWhileLoop)
        as->Label(labelStartOverAgain);

    // Test all binary clauses:
    NodeBinaryClause* bn = dynamic_cast<NodeBinaryClause*>(node->m_binaryClause);


        QString failedLabel = labelElseDone;
        if (node->m_elseBlock!=nullptr)
            failedLabel = labelElse;

        BuildSimple(bn,  failedLabel);

    // Start main block
    as->Label(lblstartTrueBlock); // This means skip inside

    node->m_block->Accept(this);

    if (node->m_elseBlock!=nullptr)
        as->Asm("bra " + labelElseDone);

    // If while loop, return to beginning of conditionals
    if (node->m_isWhileLoop)
        as->Asm("bra " + labelStartOverAgain);

    // An else block?
    if (node->m_elseBlock!=nullptr) {
        as->Label(labelElse);
        node->m_elseBlock->Accept(this);
//        m_elseBlock->Build(as);

    }
    as->Label(labelElseDone); // Jump here if not

    as->PopLabel("while");
    as->PopLabel("ConditionalTrueBlock");
    as->PopLabel("elseblock");
    as->PopLabel("elsedoneblock");
//    as->PopLabel("conditionalfailed");


}

void ASTDispather68000::dispatch(NodeForLoop *node)
{
    node->DispatchConstructor();


    //QString m_currentVar = ((NodeAssign*)m_a)->m_
    NodeAssign *nVar = dynamic_cast<NodeAssign*>(node->m_a);


    if (nVar==nullptr)
        ErrorHandler::e.Error("Index must be variable", node->m_op.m_lineNumber);

    QString var = dynamic_cast<NodeVar*>(nVar->m_left)->value;//  m_a->Build(as);
//    qDebug() << "Starting for";
    node->m_a->Accept(this);
  //  qDebug() << "accepted";

//    LoadVariable(node->m_a);
  //  TransformVariable()
    //QString to = m_b->Build(as);
    QString to = "";
    if (dynamic_cast<const NodeNumber*>(node->m_b) != nullptr)
        to = QString::number(((NodeNumber*)node->m_b)->m_val);
    if (dynamic_cast<const NodeVar*>(node->m_b) != nullptr)
        to = ((NodeVar*)node->m_b)->value;

//    as->m_stack["for"].push(var);
    QString lblFor =as->NewLabel("forloop");
    as->Label(lblFor);
//    qDebug() << "end for";



    node->m_block->Accept(this);
    TransformVariable(as,"add",var, "#1");
    LoadVariable(node->m_b);
    TransformVariable(as,"cmp",as->m_varStack.pop(),var);
    as->Asm("bne "+lblFor);

    as->PopLabel("forloop");


}

void ASTDispather68000::dispatch(NodeVar *node)
{
//    LoadVariable(node);
    if (node->m_expr!=nullptr) {
//        qDebug() << "HERE";
        LoadVariable(node);
        return;
    }
    as->m_varStack.push(node->getValue());
}

void ASTDispather68000::dispatch(Node *node)
{

}

void ASTDispather68000::dispatch(NodeAssign *node)
{
    node->DispatchConstructor();

//    as->PushCounter();

    AssignVariable(node);

  //  as->PopCounter(node->m_op.m_lineNumber);

}

void ASTDispather68000::dispatch(NodeBuiltinMethod *node)
{
    node->DispatchConstructor();

    node->VerifyParams(as);

  //  as->PushCounter();

    Methods68000 methods;
    methods.m_node = node;
    methods.Assemble(as,this);

//    as->PopCounter(node->m_op.m_lineNumber-1);

}

void ASTDispather68000::StoreVariable(NodeVar *n)
{
        as->Comment("StoreVar : " +QString::number(n->m_expr==nullptr));
        if (n->m_expr!=nullptr) {
    //        qDebug() << n->m_op.getType();
      //      exit(1);
            bool done = false;
            if (as->m_regAcc.m_latest.count()==2) {
                TransformVariable(as,"move.w",as->m_regAcc.m_latest,"#0");
                done = true;
            }
            QString d0 = as->m_varStack.pop();
//            QString d0 = as->m_regAcc.Get();
            QString a0 = as->m_regMem.Get();

            if (!done && d0.toLower().startsWith("d") )
                TransformVariable(as,"move.w",d0,"#0");

            //qDebug() << "Loading array: expression";
            LoadVariable(n->m_expr);
            QString d1 = as->m_varStack.pop();
            //qDebug() << "Popping varstack: " <<d1;
            TransformVariable(as,"lea",a0,n->value);
            TransformVariable(as,"move"+getEndType(as,n),"("+a0+","+d1+")",d0);
            //qDebug() << "Cleaning up loadvar: " <<d1;
    //        as->m_varStack.push(d0);
            as->m_regMem.Pop(a0);
/*            for (QString s: as->m_regAcc.m_free)
                as->Comment(" END of storevar free : " +s);
            for (QString s: as->m_regAcc.m_occupied)
                as->Comment(" END of storevar occ : " +s);
            as->Comment(" END of storevar latest : " +as->m_regAcc.m_latest);
            */
  //          as->m_regAcc.Pop(d0);
    //        as->m_regAcc.Pop(d0);
    //        LoadPointer(n);
            //qDebug() << "Done: ";

            return;
        }

        QString d0 = as->m_varStack.pop();
        TransformVariable(as,"move"+getEndType(as,n),n->getValue(),d0);
  //      as->m_regAcc.Pop(d0);
    //    as->m_varStack.push(d0);

}


void ASTDispather68000::LoadVariable(NodeVar *n)
{
//    TokenType::Type t = as->m_symTab->Lookup(n->value, n->m_op.m_lineNumber)->getTokenType();

    if (n->m_expr!=nullptr) {
//        qDebug() << n->m_op.getType();
  //      exit(1);
        bool done = false;
        if (as->m_regAcc.m_latest.count()==2) {
            TransformVariable(as,"move.w",as->m_regAcc.m_latest,"#0");
            done = true;
        }
        QString d0 = as->m_regAcc.Get();
        QString a0 = as->m_regMem.Get();
        if (!done )
            TransformVariable(as,"move.w",d0,"#0");
        //qDebug() << "Loading array: expression";
        LoadVariable(n->m_expr);
        QString d1 = as->m_varStack.pop();
        //qDebug() << "Popping varstack: " <<d1;
        TransformVariable(as,"lea",a0,n->value);
        TransformVariable(as,"move",d0,"("+a0+","+d1+")",n);
        //qDebug() << "Cleaning up loadvar: " <<d1;
        as->m_varStack.push(d0);

        as->m_regMem.Pop(a0);
        as->m_regAcc.Pop(d0);
//        as->m_regAcc.Pop(d0);
//        LoadPointer(n);
        //qDebug() << "Done: ";

        return;
    }

    QString d0 = as->m_regAcc.Get();
    TransformVariable(as,"move"+getEndType(as,n),d0,n->getValue());
    as->m_regAcc.Pop(d0);
    as->m_varStack.push(d0);
}

void ASTDispather68000::LoadVariable(Node *n)
{
//    qDebug() << "Don't call Dispatcher::LoadVariable with NODE. ";
  //  exit(1);
    NodeVar* nv = dynamic_cast<NodeVar*>(n);
    NodeNumber* nn = dynamic_cast<NodeNumber*>(n);
    if (nv!=nullptr) {

        LoadVariable((NodeVar*)n);
    }
    else
    if (nn!=nullptr){
        LoadVariable((NodeNumber*)n);
    }
    else {
        n->Accept(this);
    }
}


void ASTDispather68000::LoadVariable(NodeNumber *n)
{
    QString d0 = as->m_regAcc.Get();
    TransformVariable(as,"move",d0, n->getValue());
    as->m_regAcc.Pop(d0);
    as->m_varStack.push(d0);

}

void ASTDispather68000::TransformVariable(Assembler *as, QString op, QString n, QString val, Node *t)
{
//    as->Asm(op+getEndType(as,t) +" "+val + "," + n);
    m_lastSize = getEndType(as,t);
    as->Asm(op+getEndType(as,t) +" "+val + "," + n);

}

void ASTDispather68000::TransformVariable(Assembler* as, QString op, NodeVar *n, QString val)
{
    as->Asm(op+getEndType(as,n) +" "+val + "," + n->getValue());
    m_lastSize = getEndType(as,n);

//    qDebug() << " ** OP : " << op+getEndType(as,n) +" "+val + "," + n->getValue();
}

void ASTDispather68000::TransformVariable(Assembler *as, QString op, QString n, NodeVar *val)
{
    as->Asm(op+getEndType(as,val) +" "+val->getValue() + "," + n);
    m_lastSize = getEndType(as,val);

//
}

void ASTDispather68000::TransformVariable(Assembler* as, QString op, QString n, QString val)
{
    as->Asm(op +" "+val + "," + n);
    //qDebug() << " ** OP : " << op +" "+val + "," + n;
}

QString ASTDispather68000::getEndType(Assembler *as, Node *v) {
//    getType(as)==TokenType::LONG
    NodeVar* nv = dynamic_cast<NodeVar*>(v);
    TokenType::Type t = v->getType(as);
    if (nv!=nullptr && nv->m_expr!=nullptr) {
      //  qDebug() << nv->value;
        Symbol* s = as->m_symTab->Lookup(nv->value, v->m_op.m_lineNumber, v->isAddress());
        if (s!=nullptr) {
            t = s->m_arrayType;
//            as->Comment("IS Array");
        }
    }
    NodeNumber* n = dynamic_cast<NodeNumber*>(v);
    if (n!=nullptr) {

        return ".w";
    }

    if (t==TokenType::INTEGER)
        return ".w";
    if (t==TokenType::LONG)
        return ".l";
    if (t==TokenType::BYTE)
        return ".b";

    as->Comment("Current tokentype : "+TokenType::getType(t));

    return ".b";
}

QString ASTDispather68000::AssignVariable(NodeAssign *node) {

    NodeVar* v = (NodeVar*)dynamic_cast<const NodeVar*>(node->m_left);
    //        qDebug() << "AssignVariable: " <<v->value << " : " << TokenType::getType( v->getType(as));

    NodeNumber* num = (NodeNumber*)dynamic_cast<const NodeNumber*>(node->m_left);
    if (v==nullptr && num == nullptr)
        ErrorHandler::e.Error("Left value not variable or memory address! ");
/*    if (num!=nullptr && num->getType(as)!=TokenType::ADDRESS)
        ErrorHandler::e.Error("Left value must be either variable or memory address");
  */


/*    if (num!=nullptr) {
        as->Comment("Assigning memory location (poke replacement)");
        v = new NodeVar(num->m_op); // Create a variable copy
        v->value = num->HexValue();
        //return num->HexValue();
    }
  */

    as->Comment("Assigning single variable : " + v->value);
//    as->Comment("Varstack : " + QString::number(as->m_varStack.m_vars.count()));
  //  as->Comment("regstack : " + QString::number(as->m_regAcc.m_free.count()));
    if (node->m_right==nullptr)
        ErrorHandler::e.Error("Node assign: right hand must be expression", node->m_op.m_lineNumber);



    if (node->m_left->getType(as)==TokenType::INTEGER) {
        node->m_right->m_forceType = TokenType::INTEGER; // FORCE integer on right-hand side
    }
    if (node->m_left->getType(as)==TokenType::LONG) {
        node->m_right->m_forceType = TokenType::LONG; // FORCE integer on right-hand side
    }
    //qDebug() << "Assigning A";

    // For constant i:=i+1;
    //if (IsSimpleIncDec(v,  node))
    //    return v->value;

    //node->m_right->Accept(this);

//    if ((node->m_right->isPureVariable() && !node->m_right->isArrayIndex() ) || node->m_right->isPureNumeric() ) {

/*    if ((node->m_right->isPureVariable() && !node->m_right->isArrayIndex() ) || node->m_right->isPureNumeric() ) {
 /*       NodeVar* test = dynamic_cast<NodeVar*>(node->m_right);
        as->Comment(node->m_right->getValue());
        if (test!=nullptr)
            qDebug() << test->m_expr;
        as->Comment("Assign: is pure variable & not array index");
*/
//        TransformVariable(as, "move",v,node->m_right->getValue());
  /*
        StoreVariable(v);
        return "";
    }
*/
    // Some expression instead
    if (node->m_right->isArrayIndex()) {
        as->Comment("Assign: is Array index");
        LoadVariable((NodeVar*)node->m_right);

    }
    else
        node->m_right->Accept(this);

//    qDebug() << "AssignVariable : " << v->value;
//    QString reg = as->m_varStack.pop();

    StoreVariable(v);
    as->Comment("regacc : " +as->m_regAcc.m_latest);
    while (as->m_varStack.m_vars.count()!=0)
        as->m_varStack.pop();

//    as->Comment("Assignvariable : " +v->value + " from register " + reg);
  //  qDebug() << "Reg" << reg;
/*    if (!node->m_left->isArrayIndex())
        TransformVariable(as, "move",v, reg);
    else {

    }*/
  //  as->Comment("AssignVariable done varstack : " + QString::number(as->m_varStack.m_vars.count()));
 //   as->m_regAcc.Pop(reg);

 //   as->Term();
   // StoreVariable(v);
    return "";
}

void ASTDispather68000::IncBin(Assembler* as, NodeVarDecl *node) {
    NodeVar* v = (NodeVar*)node->m_varNode;
    NodeVarType* t = (NodeVarType*)node->m_typeNode;
    QString filename = as->m_projectDir + "/" + t->m_filename;
    if (!QFile::exists(filename))
        ErrorHandler::e.Error("Could not locate binary file for inclusion :" +filename);

    int size=0;
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)){
        size = f.size();  //when file does open.
        f.close();
    }


    if (t->m_position=="") {
        as->Asm(" 	CNOP 0,4");

        as->Label(v->value);
        as->Asm("incbin \"" + filename + "\"");
    }
    else {
        //            qDebug() << "bin: "<<v->value << " at " << t->m_position;
//        Appendix app(t->m_position);

        Symbol* typeSymbol = as->m_symTab->Lookup(v->value, node->m_op.m_lineNumber);
        typeSymbol->m_org = Util::C64StringToInt(t->m_position);
        typeSymbol->m_size = size;
        //            qDebug() << "POS: " << typeSymbol->m_org;
        //app.Append("org " +t->m_position,1);
        as->Asm(" 	CNOP 0,4");

        as->Label(v->value);
        as->Asm("incbin \"" + filename + "\"");
/*        bool ok;
        int start=0;
        if (t->m_position.startsWith("$")) {
            start = t->m_position.remove("$").toInt(&ok, 16);
        }
        else start = t->m_position.toInt();
*/
  //      as->blocks.append(new MemoryBlock(start,start+size, MemoryBlock::DATA,t->m_filename));

    }
}

void ASTDispather68000::BuildSimple(Node *node, QString lblFailed)
{

    as->Comment("Binary clause Simplified: " + node->m_op.getType());
    //    as->Asm("pha"); // Push that baby

    BuildToCmp(node);

    if (node->m_op.m_type==TokenType::EQUALS)
        as->Asm("bne " + lblFailed);
    if (node->m_op.m_type==TokenType::NOTEQUALS)
        as->Asm("beq " + lblFailed);
    if (node->m_op.m_type==TokenType::LESS)
        as->Asm("bcc " + lblFailed);
    if (node->m_op.m_type==TokenType::GREATER)
        as->Asm("bcs " + lblFailed);



}

void ASTDispather68000::BuildToCmp(Node *node)
{
//    QString b="";

  /*  NodeVar* varb = dynamic_cast<NodeVar*>(node->m_right);
    if (varb!=nullptr && varb->m_expr==nullptr)
        b = varb->value;

    NodeNumber* numb = dynamic_cast<NodeNumber*>(node->m_right);
    if (numb!=nullptr)
        b = numb->StringValue();
*/
//    as->Term();

    if (node->m_left->getValue()!="") {
        if (node->m_right->isPureNumeric())
        {
            as->Comment("Compare with pure num / var optimization");
            TransformVariable(as,"cmp",node->m_left->getValue(),node->m_right->getValue(),node->m_left);
            return;
        } else
        {
            as->Comment("Compare two vars optimization");
            if (node->m_right->isPureVariable())
                LoadVariable((NodeVar*)node->m_right);
                 else
                node->m_right->Accept(this);

            TransformVariable(as,"cmp",as->m_varStack.pop(),(NodeVar*)node->m_left);
            return;
        }
    }

    node->m_right->Accept(this);

    TransformVariable(as,"cmp",(NodeVar*)node->m_left, as->m_varStack.pop());

        // Perform a full compare : create a temp variable
//        QString tmpVar = as->m_regAcc.Get();//as->StoreInTempVar("binary_clause_temp");
//        node->m_right->Accept(this);
  //      as->Asm("cmp " + tmpVar);
    //      as->PopTempVar();


}

