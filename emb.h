/*
 * This code copied from https://github.com/mloskot/workshop
 *
 * It's a nice alternative to using the mixed Python/C++ solution
 * advocated in https://docs.python.org/3/faq/extending.html but it
 * won't work for us here, as it won't compile without some flags
 * provided to GCC. In future, when GCC 4.8 becomes standard issue, we
 * can start using this, so i'll incorporate it into the SC codebase.
 *
 * Seb James, Aug 2014.
 */

//
// Copyright (C) 2011 Mateusz Loskot <mateusz@loskot.net>
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Blog article: http://mateusz.loskot.net/?p=2819

// functional for Qt (Seb added in place of single line '#include <functional>')
#include <functional>
#if !(__cplusplus > 199711L || __GXX_EXPERIMENTAL_CXX0X__ || _MSC_VER >= 1600 || defined( _LIBCPP_VERSION ))
# define USE_TR1
#endif
#ifdef USE_TR1
# ifdef __GNUC__
#  include <tr1/functional>
# endif
namespace cpp11x = std::tr1;
#else
namespace cpp11x = std;
#endif

#include <iostream>
#include <string>
#include <Python.h>

#ifdef EXAMPLE_OF_EMB_USAGE
int main()
{
    PyImport_AppendInittab("emb", emb::PyInit_emb);
    Py_Initialize();
    PyImport_ImportModule("emb");

    PyRun_SimpleString("print(\'hello to console\')");

    // here comes the ***magic***
    std::string buffer;
    {
        // switch sys.stdout to custom handler
        emb::stdout_write_type write = [&buffer] (std::string s) { buffer += s; };
        emb::set_stdout(write);
        PyRun_SimpleString("print(\'hello to buffer\')");
        PyRun_SimpleString("print(3.14)");
        PyRun_SimpleString("print(\'still talking to buffer\')");
        emb::reset_stdout();
    }

    PyRun_SimpleString("print(\'hello to console again\')");
    Py_Finalize();

    // output what was written to buffer object
    std::clog << buffer << std::endl;
}
#endif

namespace emb
{

    typedef cpp11x::function<void(std::string)> stdout_write_type;

    struct Stdout
    {
        PyObject_HEAD
        stdout_write_type write;
    };

    PyObject* Stdout_write(PyObject* self, PyObject* args)
    {
        std::size_t written(0);
        Stdout* selfimpl = reinterpret_cast<Stdout*>(self);
        if (selfimpl->write)
        {
            char* data;
            if (!PyArg_ParseTuple(args, "s", &data))
                return 0;

            std::string str(data);
            selfimpl->write(str);
            written = str.size();
        }
        return PyLong_FromSize_t(written);
    }

    PyObject* Stdout_flush(PyObject* self, PyObject* args)
    {
        // no-op
        return Py_BuildValue("");
    }

    PyMethodDef Stdout_methods[] =
    {
        {"write", Stdout_write, METH_VARARGS, "sys.stdout.write"},
        {"flush", Stdout_flush, METH_VARARGS, "sys.stdout.write"},
        {0, 0, 0, 0} // sentinel
    };

    PyTypeObject StdoutType =
    {
        PyVarObject_HEAD_INIT(0, 0)
        "emb.StdoutType",     /* tp_name */
        sizeof(Stdout),       /* tp_basicsize */
        0,                    /* tp_itemsize */
        0,                    /* tp_dealloc */
        0,                    /* tp_print */
        0,                    /* tp_getattr */
        0,                    /* tp_setattr */
        0,                    /* tp_reserved */
        0,                    /* tp_repr */
        0,                    /* tp_as_number */
        0,                    /* tp_as_sequence */
        0,                    /* tp_as_mapping */
        0,                    /* tp_hash  */
        0,                    /* tp_call */
        0,                    /* tp_str */
        0,                    /* tp_getattro */
        0,                    /* tp_setattro */
        0,                    /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,   /* tp_flags */
        "emb.Stdout objects", /* tp_doc */
        0,                    /* tp_traverse */
        0,                    /* tp_clear */
        0,                    /* tp_richcompare */
        0,                    /* tp_weaklistoffset */
        0,                    /* tp_iter */
        0,                    /* tp_iternext */
        Stdout_methods,       /* tp_methods */
        0,                    /* tp_members */
        0,                    /* tp_getset */
        0,                    /* tp_base */
        0,                    /* tp_dict */
        0,                    /* tp_descr_get */
        0,                    /* tp_descr_set */
        0,                    /* tp_dictoffset */
        0,                    /* tp_init */
        0,                    /* tp_alloc */
        0,                    /* tp_new */
    };

    // Here, Seb gets compiler error: PyModuleDef does not name a type.
    PyModuleDef embmodule =
    {
        PyModuleDef_HEAD_INIT,
        "emb", 0, -1, 0,
    };

    // Internal state
    PyObject* g_stdout;
    PyObject* g_stdout_saved;

    PyMODINIT_FUNC PyInit_emb(void)
    {
        g_stdout = 0;
        g_stdout_saved = 0;

        StdoutType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&StdoutType) < 0)
            return 0;

        PyObject* m = PyModule_Create(&embmodule);
        if (m)
        {
            Py_INCREF(&StdoutType);
            PyModule_AddObject(m, "Stdout", reinterpret_cast<PyObject*>(&StdoutType));
        }
        return m;
    }

    void set_stdout(stdout_write_type write)
    {
        if (!g_stdout)
        {
            g_stdout_saved = PySys_GetObject("stdout"); // borrowed
            g_stdout = StdoutType.tp_new(&StdoutType, 0, 0);
        }

        Stdout* impl = reinterpret_cast<Stdout*>(g_stdout);
        impl->write = write;
        PySys_SetObject("stdout", g_stdout);
    }

    void reset_stdout()
    {
        if (g_stdout_saved)
            PySys_SetObject("stdout", g_stdout_saved);

        Py_XDECREF(g_stdout);
        g_stdout = 0;
    }

} // namespace emb
