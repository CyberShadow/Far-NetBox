/*
Авторские права (c) февраль 2012,
     Константин Масленников  (mkgorel2@gmail.ru),
     Сергей Харченко         (sx80@mail.ru). 

Данная лицензия разрешает лицам, получившим копию данного  программного обеспечения и  сопутствующей
документации  (в  дальнейшем  именуемыми  «Программное  Обеспечение»),   безвозмездно   использовать 
Программное Обеспечение без ограничений, включая неограниченное право на использование, копирование,
изменение,  добавление,  публикацию,  распространение,   сублицензирование   и/или   продажу   копий
Программного Обеспечения, также как и лицам, которым предоставляется данное Программное Обеспечение,
при соблюдении следующих условий:

Указанное выше уведомление об авторском праве и данные условия должны быть включены во все копии или
значимые части данного Программного Обеспечения.

ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, ЯВНО  ВЫРАЖЕННЫХ
ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ, НО НЕ ОГРАНИЧИВАЯСЬ ГАРАНТИЯМИ ТОВАРНОЙ  ПРИГОДНОСТИ, СООТВЕТСТВИЯ  ПО 
ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ НАРУШЕНИЙ ПРАВ. НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ 
НЕ НЕСУТ ОТВЕТСТВЕННОСТИ ПО ИСКАМ О ВОЗМЕЩЕНИИ УЩЕРБА, УБЫТКОВ ИЛИ ДРУГИХ ТРЕБОВАНИЙ ПО  ДЕЙСТВУЮЩИМ
КОНТРАКТАМ,  ДЕЛИКТАМ  ИЛИ  ИНОМУ,  ВОЗНИКШИМ  ИЗ,  ИМЕЮЩИМ  ПРИЧИНОЙ  ИЛИ  СВЯЗАННЫМ С  ПРОГРАММНЫМ
ОБЕСПЕЧЕНИЕМ  ИЛИ  ИСПОЛЬЗОВАНИЕМ  ПРОГРАММНОГО  ОБЕСПЕЧЕНИЯ  ИЛИ  ИНЫМИ  ДЕЙСТВИЯМИ  С  ПРОГРАММНЫМ 
ОБЕСПЕЧЕНИЕМ.
*/

#ifndef PP_PREPROCESSOR_HPP_
#define PP_PREPROCESSOR_HPP_

#if defined(_MSC_VER)
  #define PI_APPLY_EX 1
#else //defined(__GNUC__) || defined(__CODEGEARC__)
  #define PI_APPLY_EX 0
#endif

#define PP_COMMA()                                ,
#define PP_OB()                                   (
#define PP_CB()                                   )
#define PP_STR(x)                                 PZ_STR(,x)
#define PP_CAT(x,y)                               PZ_CAT(x,y)

#define PP_INC(n)                                 PD_INFO(INC, PD_N_INFO(n))
#define PP_DEC(n)                                 PD_INFO(DEC, PD_N_INFO(n))

#define PP_SEQ_SIZE(_,seq)                        PD_INFO(VAL, PD_SEQ_INFO(,seq))
#define PP_SEQ_POP_FRONT(seq)                     PD_APPLY(PZ_SEQ_POP_FRONT seq)
#define PP_SEQ_FRONT(seq)                         PZ_SEQ_FRONT(PZ_SEQ_FRONT_I seq)
#define PP_SEQ_TO_VEC(_,seq)                      (PP_SEQ_SIZE(,seq), seq)

#define PP_TPL(n,tuple)                           PD_INVOKE(PP_CAT(PZ_VA,n), tuple)
#define PP_TPL_SIZE(tuple)                        PD_INVOKE(PZ_VA_SIZE, tuple)
#define PP_TPL_TO_ARGS(tuple)                     PD_INVOKE(PP_CAT(PZ_VA_TO_ARGS, PP_TPL_SIZE(tuple)), tuple)
#define PP_TPL_TO_VEC(tuple)                      PD_INVOKE(PP_CAT(PZ_VA_TO_VEC, PP_TPL_SIZE(tuple)), tuple)
#define PP_TPL_MACRO(tuple,macro,data)            PP_TPL_MACRO_R(PD_R)(tuple,macro,data)
#define PP_TPL_MACRO_R(r)/*(tuple,macro,data)*/   PP_CAT(PZ_TPL_MACRO_R, PP_TPL(1,r))

#define PP_VEC_SIZE(vec)                          PP_TPL(1, vec)
#define PP_VEC_DATA(vec)                          PP_TPL(2, vec)

#define PP_TPLSEQ_TO_VEC(_,tplseq)                         PP_SEQ_TO_VEC(, PP_CAT(PZ_TPLSEQ_TO_VEC_I tplseq, L) () )

// flags: I, FI, IL, FIL
#define PP_VEC_FOR(vec,macro,data,flags)                   PP_VEC_FOR_R(PD_R)(PD_R,vec,macro,data,flags)
#define PP_VEC_FOR_R(r)/*(r,vec,macro,data,flags)*/        PP_CAT(PZ_VEC_FOR_R, PP_TPL(1,r))
#define PP_VEC_ENUM(vec,macro,data,flags)                  PP_VEC_ENUM_R(PD_R)(PD_R,vec,macro,data,flags)
#define PP_VEC_ENUM_R(r)/*(r,vec,macro,data,flags)*/       PP_CAT(PZ_VEC_ENUM_R, PP_TPL(1,r))

#define PP_SEQ_FOR(seq,macro,data,flags)                   PP_SEQ_FOR_R(PD_R)(PD_R,seq,macro,data,flags)
#define PP_SEQ_FOR_R(r)/*(r,seq,macro,data,flags)*/        PP_CAT(PZ_SEQ_FOR_R, PP_TPL(1,r))
#define PP_SEQ_ENUM(seq,macro,data,flags)                  PP_SEQ_ENUM_R(PD_R)(PD_R,seq,macro,data,flags)
#define PP_SEQ_ENUM_R(r)/*(r,seq,macro,data,flags)*/       PP_CAT(PZ_SEQ_ENUM_R, PP_TPL(1,r))

#define PP_TPLVEC_FOR(tplvec,macro,data,flags)             PP_TPLVEC_FOR_R(PD_R)(PD_R,tplvec,macro,data,flags)
#define PP_TPLVEC_FOR_R(r)/*(tplvec,macro,data,flags)*/    PP_CAT(PZ_TPLVEC_FOR_R, PP_TPL(1,r))
#define PP_TPLVEC_ENUM(tplvec,macro,data,flags)            PP_TPLVEC_ENUM_R(PD_R)(PD_R,tplvec,macro,data,flags)
#define PP_TPLVEC_ENUM_R(r)/*(tplvec,macro,data,flags)*/   PP_CAT(PZ_TPLVEC_ENUM_R, PP_TPL(1,r))

#define PP_TPLSEQ_FOR(tplseq,macro,data,flags)             PP_TPLSEQ_FOR_R(PD_R)(PD_R,tplseq,macro,data,flags)
#define PP_TPLSEQ_FOR_R(r)/*(tplseq,macro,data,flags)*/    PP_CAT(PZ_TPLSEQ_FOR_R, PP_TPL(1,r))
#define PP_TPLSEQ_ENUM(tplseq,macro,data,flags)            PP_TPLSEQ_ENUM_R(PD_R)(PD_R,tplseq,macro,data,flags)
#define PP_TPLSEQ_ENUM_R(r)/*(tplseq,macro,data,flags)*/   PP_CAT(PZ_TPLSEQ_ENUM_R, PP_TPL(1,r))

#define PP_REPEAT(n,macro,data)                            PP_REPEAT_R(PD_R)(PD_R,n,macro,data)
#define PP_REPEAT_R(r)/*(r,macro,data)*/                   PP_CAT(PZ_REPEAT_R, PP_TPL(1,r))
#define PP_REPEAT_ENUM(n,macro,data)                       PP_REPEAT_ENUM_R(PD_R)(PD_R,n,macro,data)
#define PP_REPEAT_ENUM_R(r)/*(r,macro,data)*/              PP_CAT(PZ_REPEAT_ENUM_R, PP_TPL(1,r))

#define PP_APPLY_R(r)/*(x)*/                      PP_CAT(PZ_P_APPLY_R , PP_TPL(1,r))
#define PP_INVOKE_R(r)/*(macro,args)*/            PP_CAT(PZ_P_INVOKE_R, PP_TPL(1,r))
#if PI_APPLY_EX
  #define PP_APPLY(x)                             PZ_P_APPLY_I_R1(x)
  #define PP_INVOKE(macro,args)                   PZ_P_APPLY_I_R1(macro args)
#else
  #define PP_APPLY(x)                             x
  #define PP_INVOKE(macro,args)                   macro args
#endif

//----- IMPLEMENTATION -----

#if PI_APPLY_EX
  #define PD_APPLY(x)           PZ_D_APPLY(x)
  #define PD_INVOKE(macro,args) PZ_D_APPLY(macro args)
#else
  #define PD_APPLY(x)           x
  #define PD_INVOKE(macro,args) macro args
#endif
#define PD_INVOKE_R(r)/*(x)*/ PP_CAT(PZ_D_INVOKE_R, PP_TPL(1,r))

#define PD_INFO(name,info) PD_APPLY(PZ_INFO_##name info)
#define PD_SEQ_INFO(_,seq) PD_APPLY(PP_CAT(PZ_INFO0 seq, L))
#define PD_N_INFO(N) PD_APPLY(PZ_INFO##N##L)
#define PD_R (1,1)

// details

#define PZ_CAT(x,y) x##y
#define PZ_STR(_,x) #x
#if PI_APPLY_EX
  #define PZ_D_APPLY(x) x
  #define PZ_P_APPLY_R1(x) PZ_P_APPLY_I_R1(x)
  #define PZ_P_APPLY_R2(x) PZ_P_APPLY_I_R2(x)
  #define PZ_P_APPLY_R3(x) PZ_P_APPLY_I_R3(x)
  #define PZ_P_APPLY_R4(x) PZ_P_APPLY_I_R4(x)
  #define PZ_P_INVOKE_R1(m,d) PZ_P_APPLY_I_R1(m d)
  #define PZ_P_INVOKE_R2(m,d) PZ_P_APPLY_I_R2(m d)
  #define PZ_P_INVOKE_R3(m,d) PZ_P_APPLY_I_R3(m d)
  #define PZ_P_INVOKE_R4(m,d) PZ_P_APPLY_I_R4(m d)
  #define PZ_P_APPLY_I_R1(x) x
  #define PZ_P_APPLY_I_R2(x) x
  #define PZ_P_APPLY_I_R3(x) x
  #define PZ_P_APPLY_I_R4(x) x
  #define PZ_D_INVOKE_R1(m,d) PZ_D_APPLY_I_R1(m d)
  #define PZ_D_INVOKE_R2(m,d) PZ_D_APPLY_I_R2(m d)
  #define PZ_D_INVOKE_R3(m,d) PZ_D_APPLY_I_R3(m d)
  #define PZ_D_INVOKE_R4(m,d) PZ_D_APPLY_I_R4(m d)
  #define PZ_D_APPLY_I_R1(x) x
  #define PZ_D_APPLY_I_R2(x) x
  #define PZ_D_APPLY_I_R3(x) x
  #define PZ_D_APPLY_I_R4(x) x
#else
  #define PZ_P_APPLY_R1(x) x
  #define PZ_P_APPLY_R2(x) x
  #define PZ_P_APPLY_R3(x) x
  #define PZ_P_APPLY_R4(x) x
  #define PZ_P_INVOKE_R1(m,d) m d
  #define PZ_P_INVOKE_R2(m,d) m d
  #define PZ_P_INVOKE_R3(m,d) m d
  #define PZ_P_INVOKE_R4(m,d) m d
  #define PZ_D_INVOKE_R1(m,d) m d
  #define PZ_D_INVOKE_R2(m,d) m d
  #define PZ_D_INVOKE_R3(m,d) m d
  #define PZ_D_INVOKE_R4(m,d) m d
#endif

#define PZ_SEQ_POP_FRONT(x)
#define PZ_SEQ_FRONT(x) PP_TPL(1,(x))
#define PZ_SEQ_FRONT_I(x) x, PZ_INFO0

#define PZ_INFO_VAL(_1,_2,_3,_4,_5) _1
#define PZ_INFO_INC(_1,_2,_3,_4,_5) _2
#define PZ_INFO_DEC(_1,_2,_3,_4,_5) _3
#define PZ_INFO_01N(_1,_2,_3,_4,_5) _4
#define PZ_INFO_TF(_1,_2,_3,_4,_5) _5

#define PZ_INFO0(x) PZ_INFO1
#define PZ_INFO1(x) PZ_INFO2
#define PZ_INFO2(x) PZ_INFO3
#define PZ_INFO3(x) PZ_INFO4
#define PZ_INFO4(x) PZ_INFO5
#define PZ_INFO5(x) PZ_INFO6
#define PZ_INFO6(x) PZ_INFO7
#define PZ_INFO7(x) PZ_INFO8
#define PZ_INFO8(x) PZ_INFO9
#define PZ_INFO9(x) PZ_INFO10
#define PZ_INFO10(x) PZ_INFO11
#define PZ_INFO11(x) PZ_INFO12
#define PZ_INFO12(x) PZ_INFO13
#define PZ_INFO13(x) PZ_INFO14
#define PZ_INFO14(x) PZ_INFO15
#define PZ_INFO15(x) PZ_INFO16
#define PZ_INFO16(x) PZ_INFO17
#define PZ_INFO17(x) PZ_INFO18
#define PZ_INFO18(x) PZ_INFO19
#define PZ_INFO19(x) PZ_INFO20
#define PZ_INFO20(x) PZ_INFO21
#define PZ_INFO21(x) PZ_INFO22
#define PZ_INFO22(x) PZ_INFO23
#define PZ_INFO23(x) PZ_INFO24
#define PZ_INFO24(x) PZ_INFO25
#define PZ_INFO25(x) PZ_INFO26
#define PZ_INFO26(x) PZ_INFO27
#define PZ_INFO27(x) PZ_INFO28
#define PZ_INFO28(x) PZ_INFO29
#define PZ_INFO29(x) PZ_INFO30

#define PZ_INFO0L (0,1,-1,0,F)
#define PZ_INFO1L (1,2,0,1,T)
#define PZ_INFO2L (2,3,1,N,T)
#define PZ_INFO3L (3,4,2,N,T)
#define PZ_INFO4L (4,5,3,N,T)
#define PZ_INFO5L (5,6,4,N,T)
#define PZ_INFO6L (6,7,5,N,T)
#define PZ_INFO7L (7,8,6,N,T)
#define PZ_INFO8L (8,9,7,N,T)
#define PZ_INFO9L (9,10,8,N,T)
#define PZ_INFO10L (10,11,9,N,T)
#define PZ_INFO11L (11,12,10,N,T)
#define PZ_INFO12L (12,13,11,N,T)
#define PZ_INFO13L (13,14,12,N,T)
#define PZ_INFO14L (14,15,13,N,T)
#define PZ_INFO15L (15,16,14,N,T)
#define PZ_INFO16L (16,17,15,N,T)
#define PZ_INFO17L (17,18,16,N,T)
#define PZ_INFO18L (18,19,17,N,T)
#define PZ_INFO19L (19,20,18,N,T)
#define PZ_INFO20L (20,21,19,N,T)
#define PZ_INFO21L (21,22,20,N,T)
#define PZ_INFO22L (22,23,21,N,T)
#define PZ_INFO23L (23,24,22,N,T)
#define PZ_INFO24L (24,25,23,N,T)
#define PZ_INFO25L (25,26,24,N,T)
#define PZ_INFO26L (26,27,25,N,T)
#define PZ_INFO27L (27,28,26,N,T)
#define PZ_INFO28L (28,29,27,N,T)
#define PZ_INFO29L (29,30,28,N,T)
#define PZ_INFO30L (30,31,29,N,T)

#define PZ_VA1(_1,...) _1
#define PZ_VA2(_1,_2,...) _2
#define PZ_VA3(_1,_2,_3,...) _3
#define PZ_VA4(_1,_2,_3,_4,...) _4
#define PZ_VA5(_1,_2,_3,_4,_5,...) _5
#define PZ_VA6(_1,_2,_3,_4,_5,_6,...) _6
#define PZ_VA7(_1,_2,_3,_4,_5,_6,_7,...) _7
#define PZ_VA8(_1,_2,_3,_4,_5,_6,_7,_8,...) _8
#define PZ_VA9(_1,_2,_3,_4,_5,_6,_7,_8,_9,...) _9
#define PZ_VA10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,...) _10

#define PZ_VA_TO_VEC0() (0, )
#define PZ_VA_TO_VEC1(_1) (1, (_1))
#define PZ_VA_TO_VEC2(_1,_2) (2, (_1)(_2))
#define PZ_VA_TO_VEC3(_1,_2,_3) (3, (_1)(_2)(_3))
#define PZ_VA_TO_VEC4(_1,_2,_3,_4) (4, (_1)(_2)(_3)(_4))
#define PZ_VA_TO_VEC5(_1,_2,_3,_4,_5) (5, (_1)(_2)(_3)(_4)(_5))
#define PZ_VA_TO_VEC6(_1,_2,_3,_4,_5,_6) (6, (_1)(_2)(_3)(_4)(_5)(_6))
#define PZ_VA_TO_VEC7(_1,_2,_3,_4,_5,_6,_7) (7, (_1)(_2)(_3)(_4)(_5)(_6)(_7))
#define PZ_VA_TO_VEC8(_1,_2,_3,_4,_5,_6,_7,_8) (8, (_1)(_2)(_3)(_4)(_5)(_6)(_7)(_8))
#define PZ_VA_TO_VEC9(_1,_2,_3,_4,_5,_6,_7,_8,_9) (9, (_1)(_2)(_3)(_4)(_5)(_6)(_7)(_8)(_9))
#define PZ_VA_TO_VEC10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10) (10, (_1)(_2)(_3)(_4)(_5)(_6)(_7)(_8)(_9)(_10))

#define PZ_VA_TO_ARGS0() 
#define PZ_VA_TO_ARGS1(_1) , _1
#define PZ_VA_TO_ARGS2(_1,_2) , _1, _2
#define PZ_VA_TO_ARGS3(_1,_2,_3) , _1, _2, _3
#define PZ_VA_TO_ARGS4(_1,_2,_3,_4) , _1, _2, _3, _4
#define PZ_VA_TO_ARGS5(_1,_2,_3,_4,_5) , _1, _2, _3, _4, _5
#define PZ_VA_TO_ARGS6(_1,_2,_3,_4,_5,_6) , _1, _2, _3, _4, _5, _6
#define PZ_VA_TO_ARGS7(_1,_2,_3,_4,_5,_6,_7) , _1, _2, _3, _4, _5, _6, _7
#define PZ_VA_TO_ARGS8(_1,_2,_3,_4,_5,_6,_7,_8) , _1, _2, _3, _4, _5, _6, _7, _8
#define PZ_VA_TO_ARGS9(_1,_2,_3,_4,_5,_6,_7,_8,_9) , _1, _2, _3, _4, _5, _6, _7, _8, _9
#define PZ_VA_TO_ARGS10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10) , _1, _2, _3, _4, _5, _6, _7, _8, _9, _10

#define PZ_VA_SIZE(...) PP_CAT(PZ_VA_SIZE_, PZ_VA_GET(__VA_ARGS__,N,N,N,N,N,N,N,N,N,01,_) )(,__VA_ARGS__) 
#define PZ_VA_SIZE_N(_,...) PZ_VA_GET(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1)
#define PZ_VA_SIZE_01(_,x) PZ_VA_GET( PZ_VA_INVOKE( PZ_VA_SPEC_B, PZ_VA_INVOKE(PZ_VA_SPEC_A, x (N1)) PZ_VA_NONE (N0)),0,1,1,1,1,1,1,1,1,1,__)
#define PZ_VA_SPEC_A(...)
#define PZ_VA_SPEC_B(x) PP_CAT(PZ_VA_SPEC_, x)
#define PZ_VA_SPEC_N0 1,2,3,4,5,6,7,8,9,10
#define PZ_VA_GET(...) PZ_VA_INVOKE( PZ_VA_GET_A, (__VA_ARGS__) )
#define PZ_VA_GET_A(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,R,...) R
#define PZ_VA_NONE
#if PI_APPLY_EX
  #define PZ_VA_INVOKE(m,a) PZ_VA_INVOKE_A( m a )
  #define PZ_VA_INVOKE_A(x) x
#else
  #define PZ_VA_INVOKE(m,a) m a
#endif

#define PZ_TPL_MACRO_R1(tuple,macro,data) PZ_TPL_MACRO_I_R1( PP_CAT(macro, PP_TPL_SIZE(tuple)), (data PP_TPL_TO_ARGS(tuple)) )
#define PZ_TPL_MACRO_R2(tuple,macro,data) PZ_TPL_MACRO_I_R2( PP_CAT(macro, PP_TPL_SIZE(tuple)), (data PP_TPL_TO_ARGS(tuple)) )
#define PZ_TPL_MACRO_R3(tuple,macro,data) PZ_TPL_MACRO_I_R3( PP_CAT(macro, PP_TPL_SIZE(tuple)), (data PP_TPL_TO_ARGS(tuple)) )
#define PZ_TPL_MACRO_R4(tuple,macro,data) PZ_TPL_MACRO_I_R4( PP_CAT(macro, PP_TPL_SIZE(tuple)), (data PP_TPL_TO_ARGS(tuple)) )
#if PI_APPLY_EX
  #define PZ_TPL_MACRO_I_R1(m,d) m d
  #define PZ_TPL_MACRO_I_R2(m,d) m d
  #define PZ_TPL_MACRO_I_R3(m,d) m d
  #define PZ_TPL_MACRO_I_R4(m,d) m d
#else
  #define PZ_TPL_MACRO_I_R1(m,d) PZ_TPL_MACRO_II_R1(m d)
  #define PZ_TPL_MACRO_I_R2(m,d) PZ_TPL_MACRO_II_R2(m d)
  #define PZ_TPL_MACRO_I_R3(m,d) PZ_TPL_MACRO_II_R3(m d)
  #define PZ_TPL_MACRO_I_R4(m,d) PZ_TPL_MACRO_II_R4(m d)
  #define PZ_TPL_MACRO_II_R1(x) x
  #define PZ_TPL_MACRO_II_R2(x) x
  #define PZ_TPL_MACRO_II_R3(x) x
  #define PZ_TPL_MACRO_II_R4(x) x
#endif

#define PZ_TPLSEQ_TO_VEC_I(...)  (( __VA_ARGS__ )) PZ_TPLSEQ_TO_VEC_II
#define PZ_TPLSEQ_TO_VEC_II(...) (( __VA_ARGS__ )) PZ_TPLSEQ_TO_VEC_I
#define PZ_TPLSEQ_TO_VEC_IL()
#define PZ_TPLSEQ_TO_VEC_IIL()

#define PZ_VEC_FOR_R1(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,1,PP_VEC_SIZE(vec),1,PZ_VEC_FOR_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_VEC_FOR_R2(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,2,PP_VEC_SIZE(vec),1,PZ_VEC_FOR_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_VEC_FOR_R3(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,3,PP_VEC_SIZE(vec),1,PZ_VEC_FOR_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_VEC_FOR_EXPR_R1(t,r,ni,i,d)  PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d))  ( (2,r), i, PP_TPL(3,d), PP_SEQ_FRONT(PP_TPL(1,d)) )
#define PZ_VEC_FOR_EXPR_R2(t,r,ni,i,d)  PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d))  ( (3,r), i, PP_TPL(3,d), PP_SEQ_FRONT(PP_TPL(1,d)) )
#define PZ_VEC_FOR_EXPR_R3(t,r,ni,i,d)  PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d))  ( (4,r), i, PP_TPL(3,d), PP_SEQ_FRONT(PP_TPL(1,d)) )
#define PZ_VEC_FOR_NEXT(d) (PP_SEQ_POP_FRONT(PP_TPL(1,d)), PP_TPL(2,d), PP_TPL(3,d), PP_TPL(4,d) )

#define PZ_VEC_ENUM_R1(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,1,PP_VEC_SIZE(vec),1,PZ_VEC_ENUM_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_VEC_ENUM_R2(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,2,PP_VEC_SIZE(vec),1,PZ_VEC_ENUM_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_VEC_ENUM_R3(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,3,PP_VEC_SIZE(vec),1,PZ_VEC_ENUM_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_VEC_ENUM_EXPR_R1(t,r,ni,i,d)  PP_CAT(PZ_ENUM_COMMA_,t)() PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d))  ( (2,r), i, PP_TPL(3,d), PP_SEQ_FRONT(PP_TPL(1,d)) )
#define PZ_VEC_ENUM_EXPR_R2(t,r,ni,i,d)  PP_CAT(PZ_ENUM_COMMA_,t)() PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d))  ( (3,r), i, PP_TPL(3,d), PP_SEQ_FRONT(PP_TPL(1,d)) )
#define PZ_VEC_ENUM_EXPR_R3(t,r,ni,i,d)  PP_CAT(PZ_ENUM_COMMA_,t)() PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d))  ( (4,r), i, PP_TPL(3,d), PP_SEQ_FRONT(PP_TPL(1,d)) )

#define PZ_SEQ_FOR_R1(r,seq,macro,data,flags) PZ_VEC_FOR_R1(r,PP_SEQ_TO_VEC(,seq),macro,data,flags)
#define PZ_SEQ_FOR_R2(r,seq,macro,data,flags) PZ_VEC_FOR_R2(r,PP_SEQ_TO_VEC(,seq),macro,data,flags)
#define PZ_SEQ_FOR_R3(r,seq,macro,data,flags) PZ_VEC_FOR_R3(r,PP_SEQ_TO_VEC(,seq),macro,data,flags)

#define PZ_SEQ_ENUM_R1(r,seq,macro,data,flags) PZ_VEC_ENUM_R1(r,PP_SEQ_TO_VEC(,seq),macro,data,flags)
#define PZ_SEQ_ENUM_R2(r,seq,macro,data,flags) PZ_VEC_ENUM_R2(r,PP_SEQ_TO_VEC(,seq),macro,data,flags)
#define PZ_SEQ_ENUM_R3(r,seq,macro,data,flags) PZ_VEC_ENUM_R3(r,PP_SEQ_TO_VEC(,seq),macro,data,flags)

#define PZ_REPEAT_R1(r,n,macro,data)  PZ_SELECT_FOR(r,n)  (F,1,n,1,PZ_REPEAT_EXPR_R,PZ_REPEAT_NEXT,(macro,data))
#define PZ_REPEAT_R2(r,n,macro,data)  PZ_SELECT_FOR(r,n)  (F,2,n,1,PZ_REPEAT_EXPR_R,PZ_REPEAT_NEXT,(macro,data))
#define PZ_REPEAT_R3(r,n,macro,data)  PZ_SELECT_FOR(r,n)  (F,3,n,1,PZ_REPEAT_EXPR_R,PZ_REPEAT_NEXT,(macro,data))
#define PZ_REPEAT_EXPR_R1(t,r,ni,i,d)  PP_TPL(1,d) ( (2,r), i, PP_TPL(2,d) )
#define PZ_REPEAT_EXPR_R2(t,r,ni,i,d)  PP_TPL(1,d) ( (3,r), i, PP_TPL(2,d) )
#define PZ_REPEAT_EXPR_R3(t,r,ni,i,d)  PP_TPL(1,d) ( (4,r), i, PP_TPL(2,d) )
#define PZ_REPEAT_NEXT(d) d

#define PZ_REPEAT_ENUM_R1(r,n,macro,data)  PZ_SELECT_FOR(r,n)  (F,1,n,1,PZ_REPEAT_ENUM_EXPR_R,PZ_REPEAT_NEXT,(macro,data))
#define PZ_REPEAT_ENUM_R2(r,n,macro,data)  PZ_SELECT_FOR(r,n)  (F,2,n,1,PZ_REPEAT_ENUM_EXPR_R,PZ_REPEAT_NEXT,(macro,data))
#define PZ_REPEAT_ENUM_R3(r,n,macro,data)  PZ_SELECT_FOR(r,n)  (F,3,n,1,PZ_REPEAT_ENUM_EXPR_R,PZ_REPEAT_NEXT,(macro,data))
#define PZ_REPEAT_ENUM_EXPR_R1(t,r,ni,i,d)  PP_CAT(PZ_ENUM_COMMA_,t)() PP_TPL(1,d) ( (2,r), i, PP_TPL(2,d) )
#define PZ_REPEAT_ENUM_EXPR_R2(t,r,ni,i,d)  PP_CAT(PZ_ENUM_COMMA_,t)() PP_TPL(1,d) ( (3,r), i, PP_TPL(2,d) )
#define PZ_REPEAT_ENUM_EXPR_R3(t,r,ni,i,d)  PP_CAT(PZ_ENUM_COMMA_,t)() PP_TPL(1,d) ( (4,r), i, PP_TPL(2,d) )

#define PZ_TPLSEQ_FOR_R1(r,tplseq,macro,data,flags)  PZ_TPLVEC_FOR_R1(r,PP_TPLSEQ_TO_VEC(,tplseq),macro,data,flags)
#define PZ_TPLSEQ_FOR_R2(r,tplseq,macro,data,flags)  PZ_TPLVEC_FOR_R2(r,PP_TPLSEQ_TO_VEC(,tplseq),macro,data,flags)
#define PZ_TPLSEQ_FOR_R3(r,tplseq,macro,data,flags)  PZ_TPLVEC_FOR_R3(r,PP_TPLSEQ_TO_VEC(,tplseq),macro,data,flags)

#define PZ_TPLSEQ_ENUM_R1(r,tplseq,macro,data,flags)  PZ_TPLVEC_ENUM_R1(r,PP_TPLSEQ_TO_VEC(,tplseq),macro,data,flags)
#define PZ_TPLSEQ_ENUM_R2(r,tplseq,macro,data,flags)  PZ_TPLVEC_ENUM_R2(r,PP_TPLSEQ_TO_VEC(,tplseq),macro,data,flags)
#define PZ_TPLSEQ_ENUM_R3(r,tplseq,macro,data,flags)  PZ_TPLVEC_ENUM_R3(r,PP_TPLSEQ_TO_VEC(,tplseq),macro,data,flags)

#define PZ_TPLVEC_FOR_R1(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,1,PP_VEC_SIZE(vec),1,PZ_TPLVEC_FOR_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_TPLVEC_FOR_R2(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,2,PP_VEC_SIZE(vec),1,PZ_TPLVEC_FOR_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_TPLVEC_FOR_R3(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,3,PP_VEC_SIZE(vec),1,PZ_TPLVEC_FOR_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_TPLVEC_FOR_EXPR_R1(t,r,ni,i,d)  PD_INVOKE_R((2,r)) ( PP_CAT(PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d)),PP_TPL_SIZE(PP_SEQ_FRONT(PP_TPL(1,d)))),  ( (2,r), i, PP_TPL(3,d) PP_TPL_TO_ARGS(PP_SEQ_FRONT(PP_TPL(1,d))) ) )
#define PZ_TPLVEC_FOR_EXPR_R2(t,r,ni,i,d)  PD_INVOKE_R((3,r)) ( PP_CAT(PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d)),PP_TPL_SIZE(PP_SEQ_FRONT(PP_TPL(1,d)))),  ( (3,r), i, PP_TPL(3,d) PP_TPL_TO_ARGS(PP_SEQ_FRONT(PP_TPL(1,d))) ) )
#define PZ_TPLVEC_FOR_EXPR_R3(t,r,ni,i,d)  PD_INVOKE_R((4,r)) ( PP_CAT(PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d)),PP_TPL_SIZE(PP_SEQ_FRONT(PP_TPL(1,d)))),  ( (4,r), i, PP_TPL(3,d) PP_TPL_TO_ARGS(PP_SEQ_FRONT(PP_TPL(1,d))) ) )

#define PZ_TPLVEC_ENUM_R1(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,1,PP_VEC_SIZE(vec),1,PZ_TPLVEC_ENUM_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_TPLVEC_ENUM_R2(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,2,PP_VEC_SIZE(vec),1,PZ_TPLVEC_ENUM_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_TPLVEC_ENUM_R3(r,vec,macro,data,flags)  PZ_SELECT_FOR(r,PP_VEC_SIZE(vec))  (F,3,PP_VEC_SIZE(vec),1,PZ_TPLVEC_ENUM_EXPR_R,PZ_VEC_FOR_NEXT,(PP_VEC_DATA(vec),macro,data,flags))
#define PZ_TPLVEC_ENUM_EXPR_R1(t,r,ni,i,d) PP_CAT(PZ_ENUM_COMMA_,t)() PD_INVOKE_R((2,r)) ( PP_CAT(PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d)),PP_TPL_SIZE(PP_SEQ_FRONT(PP_TPL(1,d)))),  ( (2,r), i, PP_TPL(3,d) PP_TPL_TO_ARGS(PP_SEQ_FRONT(PP_TPL(1,d))) ) )
#define PZ_TPLVEC_ENUM_EXPR_R2(t,r,ni,i,d) PP_CAT(PZ_ENUM_COMMA_,t)() PD_INVOKE_R((3,r)) ( PP_CAT(PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d)),PP_TPL_SIZE(PP_SEQ_FRONT(PP_TPL(1,d)))),  ( (3,r), i, PP_TPL(3,d) PP_TPL_TO_ARGS(PP_SEQ_FRONT(PP_TPL(1,d))) ) )
#define PZ_TPLVEC_ENUM_EXPR_R3(t,r,ni,i,d) PP_CAT(PZ_ENUM_COMMA_,t)() PD_INVOKE_R((4,r)) ( PP_CAT(PZ_ADD_MACRO_SUFFIX(t, ni, PP_TPL(4,d), PP_TPL(2,d)),PP_TPL_SIZE(PP_SEQ_FRONT(PP_TPL(1,d)))),  ( (4,r), i, PP_TPL(3,d) PP_TPL_TO_ARGS(PP_SEQ_FRONT(PP_TPL(1,d))) ) )

#define PZ_ENUM_COMMA_F() 
#define PZ_ENUM_COMMA_N() ,

#define PZ_ADD_MACRO_SUFFIX(t,ni,flags,macro) PP_CAT(macro,PP_CAT(PP_CAT(PP_CAT(PZ_MACRO_SUFFIX_,flags),t),PD_INFO(01N,ni)))
#define PZ_MACRO_SUFFIX_IF1 
#define PZ_MACRO_SUFFIX_IFN 
#define PZ_MACRO_SUFFIX_IN1 
#define PZ_MACRO_SUFFIX_INN 
#define PZ_MACRO_SUFFIX_FIF1 _F
#define PZ_MACRO_SUFFIX_FIFN _F
#define PZ_MACRO_SUFFIX_FIN1 
#define PZ_MACRO_SUFFIX_FINN 
#define PZ_MACRO_SUFFIX_ILF1 _L
#define PZ_MACRO_SUFFIX_ILFN 
#define PZ_MACRO_SUFFIX_ILN1 _L
#define PZ_MACRO_SUFFIX_ILNN 
#define PZ_MACRO_SUFFIX_FILF1 _S
#define PZ_MACRO_SUFFIX_FILFN _F
#define PZ_MACRO_SUFFIX_FILN1 _L
#define PZ_MACRO_SUFFIX_FILNN 

#define PZ_SELECT_FOR(r,n) PP_CAT(PZ_SELECT_FOR_, PD_INFO(TF,PD_N_INFO(n))) ( r )
#define PZ_SELECT_FOR_F(r) PZ_FOR0
#define PZ_SELECT_FOR_T(r) PP_CAT(PZ_FOR, PP_TPL(2,r))

#define PZ_NEXT_FOR(r,n) PP_CAT(PZ_NEXT_FOR_, PD_INFO(01N,PD_N_INFO(n))) ( r )
#define PZ_NEXT_FOR_1(r) PZ_FOR0
#define PZ_NEXT_FOR_N(r) PZ_FOR##r

#define PZ_FOR0(t,r,n,i,expr,next,data)
#define PZ_FOR1(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,2,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(2,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR2(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,3,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(3,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR3(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,4,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(4,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR4(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,5,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(5,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR5(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,6,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(6,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR6(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,7,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(7,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR7(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,8,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(8,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR8(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,9,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(9,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR9(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,10,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(10,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR10(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,11,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(11,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR11(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,12,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(12,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR12(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,13,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(13,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR13(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,14,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(14,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR14(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,15,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(15,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR15(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,16,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(16,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR16(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,17,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(17,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR17(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,18,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(18,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR18(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,19,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(19,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR19(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,20,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(20,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR20(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,21,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(21,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR21(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,22,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(22,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR22(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,23,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(23,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR23(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,24,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(24,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR24(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,25,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(25,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR25(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,26,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(26,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR26(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,27,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(27,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR27(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,28,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(28,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR28(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,29,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(29,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR29(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,30,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(30,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR30(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,31,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(31,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR31(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,32,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(32,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR32(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,33,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(33,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR33(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,34,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(34,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR34(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,35,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(35,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR35(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,36,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(36,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR36(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,37,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(37,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR37(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,38,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(38,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR38(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,39,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(39,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR39(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,40,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(40,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR40(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,41,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(41,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR41(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,42,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(42,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR42(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,43,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(43,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR43(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,44,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(44,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR44(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,45,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(45,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR45(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,46,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(46,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR46(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,47,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(47,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR47(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,48,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(48,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR48(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,49,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(49,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR49(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,50,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(50,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR50(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,51,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(51,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR51(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,52,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(52,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR52(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,53,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(53,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR53(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,54,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(54,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR54(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,55,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(55,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR55(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,56,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(56,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR56(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,57,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(57,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR57(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,58,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(58,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR58(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,59,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(59,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR59(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,60,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(60,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))
#define PZ_FOR60(t,r,n,i,expr,next,data)  PP_CAT(expr,r)(t,61,PD_N_INFO(n),i,data)   PZ_NEXT_FOR(61,n) (N,r,PP_DEC(n),PP_INC(i),expr,next,next(data))

#endif // PP_PREPROCESSOR_HPP_
