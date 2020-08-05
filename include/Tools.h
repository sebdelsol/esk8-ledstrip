#pragma once

#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N

#define _fe_1(G, _call, _lastcall, x) _lastcall(G, x)
#define _fe_2(G, _call, _lastcall, x, ...) _call(G, x) _fe_1(G, _call, _lastcall, __VA_ARGS__)
#define _fe_3(G, _call, _lastcall, x, ...) _call(G, x) _fe_2(G, _call, _lastcall, __VA_ARGS__)
#define _fe_4(G, _call, _lastcall, x, ...) _call(G, x) _fe_3(G, _call, _lastcall, __VA_ARGS__)
#define _fe_5(G, _call, _lastcall, x, ...) _call(G, x) _fe_4(G, _call, _lastcall, __VA_ARGS__)
#define _fe_6(G, _call, _lastcall, x, ...) _call(G, x) _fe_5(G, _call, _lastcall, __VA_ARGS__)
#define _fe_7(G, _call, _lastcall, x, ...) _call(G, x) _fe_6(G, _call, _lastcall, __VA_ARGS__)
#define _fe_8(G, _call, _lastcall, x, ...) _call(G, x) _fe_7(G, _call, _lastcall, __VA_ARGS__)
#define _fe_9(G, _call, _lastcall, x, ...) _call(G, x) _fe_8(G, _call, _lastcall, __VA_ARGS__)

#define CALL_MACRO_X_FOR_EACH_LAST(G, _mac, _lastmac, ...) _GET_NTH_ARG( __VA_ARGS__, _fe_9, _fe_8, _fe_7, _fe_6, _fe_5, _fe_4, _fe_3, _fe_2, _fe_1)(G, _mac, _lastmac, __VA_ARGS__)
#define CALL_MACRO_X_FOR_EACH(G, _mac, ...) CALL_MACRO_X_FOR_EACH_LAST(G, _mac, _mac, __VA_ARGS__)
// -----------------------------------------------------

#define _AddSpace(sep, txt) txt << sep <<
#define _LastAddSpace(sep, txt) txt
#define SEPbySPACE(...) CALL_MACRO_X_FOR_EACH_LAST(" ", _AddSpace, _LastAddSpace, __VA_ARGS__)
