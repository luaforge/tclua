/* $Id: tclua.c,v 1.1 2007-01-09 14:06:31 tclua Exp $ */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "tcl.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define LUA_ENTER(L)    int __stacksize = lua_gettop(L)
#define LUA_RETURN(L,X) assert(__stacksize == lua_gettop(L));return X

#ifdef _DEBUG
static void stackDump(lua_State *L) {
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        printf("`%s'", lua_tostring(L, i));
        break;

      case LUA_TBOOLEAN:  /* booleans */
        printf(lua_toboolean(L, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:  /* numbers */
        printf("%g", lua_tonumber(L, i));
        break;

      default:  /* other values */
        printf("%s", lua_typename(L, t));
        break;

    }
    printf("  ");  /* put a separator */
  }
  printf("\n");  /* end the listing */
}
#endif

static int luaHandleProc(ClientData data, Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[])
{
  int length;
  int error;
  char* luacode;
  lua_State* L = (lua_State*)data;
  LUA_ENTER(L);

  if(objc != 2){
    Tcl_WrongNumArgs(interp, 1, objv, "luacode");
    LUA_RETURN(L, TCL_ERROR);
  }

  luacode = Tcl_GetStringFromObj(objv[1], &length);
  error = luaL_loadbuffer(L, luacode, length,"tclua") || lua_pcall(L, 0, 0, 0);
  if (error) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj(lua_tostring(L, -1), -1));
    lua_pop(L, 1);
    LUA_RETURN(L, TCL_ERROR);
  }

  Tcl_SetObjResult(interp, Tcl_NewStringObj("", -1));
  LUA_RETURN(L, TCL_OK);
}

/*
  push_function

  return value:
    0=success, 1=fail

  side effect:
    push a function on the stack when succeeded,
    set the Tcl result when failed.
*/
static int push_function(lua_State* L, Tcl_Interp* interp, const char* funname)
{
  /* push function name */
  if (strchr(funname, '.') == NULL) {
    /* if it's a global function, just push it */
    lua_getglobal(L, funname);
  } else {
    /* if the function is contained in a module(=table) ... */
    int argc = 0;
    char** argv = NULL;
    int i;
    size_t length = strlen(funname);
    char* buf = malloc(length+256);

    sprintf(buf, "split %s .", funname);
    Tcl_Eval(interp, buf);
    Tcl_SplitList(interp, Tcl_GetStringResult(interp), &argc, &argv);

    lua_getglobal(L, argv[0]);
    for (i = 1; i < argc; i++) {
      if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, "attempt to index a non-table value: ", argv[i-1], NULL);
        return 1;
      }
      lua_getfield(L, -1, argv[i]);
      lua_remove(L, -2); /* remove the indexee table from the stack */
    }
    Tcl_Free((char *)argv);
    free(buf);
  }
  return 0;
}

static int callHandleProc(ClientData data, Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[])
{
  char* funname;
  int i;
  int argsbegin = 1;
  int resultc = 1;
  char** resultv = NULL;
  lua_State* L = (lua_State*)data;
  LUA_ENTER(L);

  if(objc < 2){
    Tcl_WrongNumArgs(interp, 1, objv, "funName ?-result varName...? ?arg...?");
    LUA_RETURN(L, TCL_ERROR);
  }

  if (strcmp(Tcl_GetString(objv[argsbegin]), "-result") == 0) {
    Tcl_SplitList(interp, Tcl_GetString(objv[argsbegin + 1]), &resultc, &resultv);
    argsbegin += 2;
  }

  funname = Tcl_GetString(objv[argsbegin]);
  if (push_function(L, interp, funname)) {
    LUA_RETURN(L, TCL_ERROR);
  }
  argsbegin++;

  /* push arguments */
  for (i = argsbegin; i < objc; i++) {
    char* arg;
    size_t length;
    arg = Tcl_GetStringFromObj(objv[i], &length);
    lua_pushlstring(L, arg, length);
  }

  /* call the function */
  if (lua_pcall(L, i - argsbegin, resultc, 0)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj(lua_tostring(L, -1), -1));
    lua_pop(L, 1);
    LUA_RETURN(L, TCL_ERROR);
  }

  if (resultv == NULL) {
    size_t length;
    const char* result = lua_tolstring(L, -1, &length);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result, -1));
    lua_pop(L, 1);
    LUA_RETURN(L, TCL_OK);
  } else {
    int i;
    for (i = resultc -1 ; i >= 0; i--) {
      size_t length;
      const char* result = lua_tolstring(L, -1, &length);
      Tcl_SetVar(interp, resultv[i], result, 0);
      lua_pop(L, 1);
    }
    Tcl_Free((char *)resultv);
    LUA_RETURN(L, TCL_OK);
  }
}

static int funexistHandleProc(ClientData data, Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[])
{
  char* funname;
  lua_State* L = (lua_State*)data;
  LUA_ENTER(L);

  if(objc != 2){
    Tcl_WrongNumArgs(interp, 1, objv, "funName");
    LUA_RETURN(L, TCL_ERROR);
  }

  funname = Tcl_GetString(objv[1]);
  if (push_function(L, interp, funname)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("0", -1));
  } else {
    if (lua_isfunction(L, -1)) {
      Tcl_SetObjResult(interp, Tcl_NewStringObj("1", -1));
    } else {
      Tcl_SetObjResult(interp, Tcl_NewStringObj("0", -1));
    }
    lua_pop(L, 1);
  }
  LUA_RETURN(L, TCL_OK);
}

void Tclua_ExitProc(ClientData clientData)
{
  lua_State* L = (lua_State*)clientData;
  lua_close(L);
}

DLLEXPORT int Tclua_Init(Tcl_Interp* interp)
{
  lua_State* L = luaL_newstate();
  Tcl_CreateExitHandler(Tclua_ExitProc, (ClientData)L);
  luaL_openlibs(L);
  Tcl_InitStubs(interp, "8.1", 0);
  Tcl_CreateObjCommand(interp, "::lua::lua", luaHandleProc, (ClientData)L, NULL);
  Tcl_CreateObjCommand(interp, "::lua::call", callHandleProc, (ClientData)L, NULL);
  Tcl_CreateObjCommand(interp, "::lua::funexist", funexistHandleProc, (ClientData)L, NULL);
  if (Tcl_Eval(interp, "namespace eval lua { namespace export * }") == TCL_ERROR) return TCL_ERROR;
  return Tcl_PkgProvide(interp, "lua", "1.00");
}