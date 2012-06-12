#include "Python.h"
#include "Python-ast.h"
#include "pyarena.h"
#include "pythonrun.h"
#include "errcode.h"
#include "marshal.h"
#include "code.h"
#include "compile.h"
#include "eval.h"
#include "osdefs.h"
#include "import.h"


static PyCodeObject *
parse_source_module(const char *pathname, FILE *fp)
{
    PyCodeObject *co = NULL;
    mod_ty mod;
    PyCompilerFlags flags;
    PyArena *arena = PyArena_New();
    if (arena == NULL)
        return NULL;
	
    flags.cf_flags = 0;
	
    mod = PyParser_ASTFromFile(fp, pathname, Py_file_input, 0, 0, &flags,
                               NULL, arena);
    if (mod) {
        co = PyAST_Compile(mod, pathname, NULL, arena);
    }
    PyArena_Free(arena);
    return co;
}

/* Load a source module from a given file and return its module
 object WITH INCREMENTED REFERENCE COUNT.  If there's a matching
 byte-compiled file, use that instead. */

static PyObject*
load_module(char *name, char *pathname, FILE *fp) {
	PyCodeObject* co = parse_source_module(pathname, fp);
	if (co == NULL)
		return NULL;
	if (Py_VerboseFlag)
		PySys_WriteStderr("import %s # from %s\n",
						  name, pathname);
	PyObject* m = PyImport_ExecCodeModuleEx(name, (PyObject *)co, pathname);
	Py_DECREF(co);
	return m;
}

/* Load a package and return its module object WITH INCREMENTED
 REFERENCE COUNT */

static PyObject *
load_package(char *name, char *pathname)
{
    PyObject *m, *d;
    PyObject *file = NULL;
    PyObject *path = NULL;
    int err;
    char buf[MAXPATHLEN+1];
    FILE *fp = NULL;
	
    m = PyImport_AddModule(name);
    if (m == NULL)
        return NULL;
    if (Py_VerboseFlag)
        PySys_WriteStderr("import %s # directory %s\n",
						  name, pathname);
    d = PyModule_GetDict(m);
    file = PyString_FromString(pathname);
    if (file == NULL)
        goto error;
    path = Py_BuildValue("[O]", file);
    if (path == NULL)
        goto error;
    err = PyDict_SetItemString(d, "__file__", file);
    if (err == 0)
        err = PyDict_SetItemString(d, "__path__", path);
    if (err != 0)
        goto error;
    buf[0] = '\0';
	strcpy(buf, pathname);
	strcat(buf, "/__init__.py");
	fp = fopen(buf, "r");
    m = load_module(name, buf, fp);
    if (fp != NULL)
        fclose(fp);
    goto cleanup;
	
error:
    m = NULL;
cleanup:
    Py_XDECREF(path);
    Py_XDECREF(file);
    return m;
}

PyMODINIT_FUNC
init_PyCrypto(void)
{
	PyObject* m = load_package("Crypto", "Crypto");
/*	PyObject* m = Py_InitModule("Crypto", NULL);
	if(!m) return;
	PyObject* d = PyModule_GetDict(m);
	
	PyDict_SetItemString
	PyImport_ImportModule()
	
	PyObject* m = Py_InitModule("_counter", NULL);
	if(!m) return;
	PyObject* d = PyModule_GetDict(m); */
//	PyImport_ImportModule
}