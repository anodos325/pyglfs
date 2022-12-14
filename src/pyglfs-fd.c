/*
 * Python language bindings for libgfapi
 *
 * Copyright (C) Andrew Walker, 2022
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <Python.h>
#include "includes.h"
#include "pyglfs.h"

static PyObject *py_glfs_fd_new(PyTypeObject *obj,
				PyObject *args_unused,
				PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = NULL;

	self = (py_glfs_fd_t *)obj->tp_alloc(obj, 0);
	if (self == NULL) {
		return NULL;
	}
	return (PyObject *)self;
}

static int py_glfs_fd_init(PyObject *obj,
			   PyObject *args,
			   PyObject *kwargs)
{
	return 0;
}

void py_glfs_fd_dealloc(py_glfs_fd_t *self)
{
	if (self->fd) {
		if (glfs_close(self->fd) == -1) {
			fprintf(stderr, "glfs_close() failed: %s",
				strerror(errno));
		}
		self->fd = NULL;
	}
	Py_CLEAR(self->parent);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject *init_glfs_fd(glfs_fd_t *fd_in, py_glfs_obj_t *hdl, int flags)
{
	py_glfs_fd_t *pyfd = NULL;

	pyfd = (py_glfs_fd_t *)PyObject_CallFunction((PyObject *)&PyGlfsFd, NULL);
	if (pyfd == NULL) {
		glfs_close(fd_in);
		return NULL;
	}

	pyfd->fd = fd_in;
	pyfd->flags = flags;
	pyfd->parent = hdl;
	Py_INCREF(hdl);

	return (PyObject *)pyfd;
}

static PyObject *py_glfs_fd_fstat(PyObject *obj,
				  PyObject *args_unused,
				  PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	struct stat st;
	int err;

	Py_BEGIN_ALLOW_THREADS
	err = glfs_fstat(self->fd, &st);
	Py_END_ALLOW_THREADS

	if (err) {
		set_glfs_exc("glfs_fstat()");
		return NULL;
	}

	return stat_to_pystat(&st);
}

static PyObject *py_glfs_fd_fsync(PyObject *obj,
				  PyObject *args_unused,
				  PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	int err;

	Py_BEGIN_ALLOW_THREADS
	err = glfs_fsync(self->fd, NULL, NULL);
	Py_END_ALLOW_THREADS

	if (err) {
		set_glfs_exc("glfs_fsync()");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *py_glfs_fd_fchdir(PyObject *obj,
				  PyObject *args_unused,
				  PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	int err;

	Py_BEGIN_ALLOW_THREADS
	err = glfs_fchdir(self->fd);
	Py_END_ALLOW_THREADS

	if (err) {
		set_glfs_exc("glfs_fchdir()");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *py_glfs_fd_fchmod(PyObject *obj,
				   PyObject *args,
				   PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	int mode = 0;
	int err;

	if (!PyArg_ParseTuple(args, "i", &mode)) {
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	err = glfs_fchmod(self->fd, mode);
	Py_END_ALLOW_THREADS

	if (err) {
		set_glfs_exc("glfs_fchmod()");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *py_glfs_fd_fchown(PyObject *obj,
				   PyObject *args,
				   PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	int uid = -1;
	int gid = -1;
	int err;

	if (!PyArg_ParseTuple(args, "ii", &uid, &gid)) {
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	err = glfs_fchown(self->fd, (uid_t)uid, (gid_t)gid);
	Py_END_ALLOW_THREADS

	if (err) {
		set_glfs_exc("glfs_fchown()");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *py_glfs_fd_ftruncate(PyObject *obj,
				      PyObject *args,
				      PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	off_t length;
	int err;

	if (!PyArg_ParseTuple(args, "L", &length)) {
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	err = glfs_ftruncate(self->fd, length, NULL, NULL);
	Py_END_ALLOW_THREADS

	if (err) {
		set_glfs_exc("glfs_ftruncate()");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *py_glfs_fd_lseek(PyObject *obj,
				  PyObject *args,
				  PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	off_t pos;
	int how = 0;
	int err;

	if (!PyArg_ParseTuple(args, "L|i", &pos, &how)) {
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	err = glfs_lseek(self->fd, pos, how);
	Py_END_ALLOW_THREADS

	if (err) {
		set_glfs_exc("glfs_lseek()");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *py_glfs_fd_pread(PyObject *obj,
				  PyObject *args,
				  PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	off_t offset;
	Py_ssize_t cnt, n;
	int flags = 0;
	PyObject *buffer = NULL;

	if (!PyArg_ParseTuple(args, "Ln", &offset, &cnt)) {
		return NULL;
	}

	if (cnt < 0) {
		errno = EINVAL;
		set_exc_from_errno("glfs_pread()");
		return NULL;
	}

	buffer = PyBytes_FromStringAndSize((char *)NULL, cnt);
	if (buffer == NULL) {
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	n = glfs_pread(self->fd, PyBytes_AS_STRING(buffer), cnt, offset, flags, NULL);
	Py_END_ALLOW_THREADS

	if (n < 0) {
		Py_DECREF(buffer);
		set_glfs_exc("glfs_pread()");
		return NULL;
	}

	if (n != cnt) {
		_PyBytes_Resize(&buffer, n);
	}

	return buffer;
}

static PyObject *py_glfs_fd_pwrite(PyObject *obj,
				   PyObject *args,
				   PyObject *kwargs_unused)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	PyObject *return_value = NULL;
	PyObject *buf = NULL;
	Py_buffer buffer = {NULL, NULL};
	off_t offset;
	Py_ssize_t _return_value;
	int flags = 0;

	if (!PyArg_ParseTuple(args, "OL", &buf, &offset)) {
		return NULL;
	}

	if (!PyObject_CheckBuffer(buf)) {
		PyErr_SetString(
                        PyExc_TypeError,
			"not a buffer."
		);
		goto cleanup;
	}

	if (PyObject_GetBuffer(buf, &buffer, PyBUF_SIMPLE) != 0) {
		goto cleanup;
	}

	if (!PyBuffer_IsContiguous(&buffer, 'C')) {
		PyErr_SetString(
                        PyExc_TypeError,
			"buffer must be contiguous."
		);
		goto cleanup;
	}

	Py_BEGIN_ALLOW_THREADS
	_return_value = glfs_pwrite(
		self->fd, buffer.buf, (size_t)buffer.len, offset, flags,
		NULL, NULL
	);
	Py_END_ALLOW_THREADS

	if (_return_value == -1) {
		set_glfs_exc("glfs_pwrite()");
	} else {
		return_value = PyLong_FromSsize_t(_return_value);
	}

cleanup:
	if (buffer.obj) {
		PyBuffer_Release(&buffer);
	}
	return return_value;
}

static PyObject *py_glfs_fd_posix_lock(PyObject *obj,
				       PyObject *args,
				       PyObject *kwargs)
{
	py_glfs_fd_t *self = (py_glfs_fd_t *)obj;
	int cmd;
	bool verbose = false;
	struct flock fl = {
		.l_type = -1,
		.l_whence = SEEK_SET,
		.l_len = 1,
		.l_start = 0,
		.l_pid = 0
	};
	const char *kwnames [] = {
		"cmd",
		"type",
		"whence",
		"start",
		"len",
		"verbose",
		NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
					 "ih|hLLb",
					 discard_const_p(char *, kwnames),
					 &cmd,
					 &fl.l_type,
					 &fl.l_whence,
					 &fl.l_start,
					 &fl.l_len,
					 &verbose)) {
		return NULL;
	}

	if ((cmd != F_GETLK) &&
	    (cmd != F_SETLK) &&
	    (cmd != F_SETLKW)) {
		PyErr_Format(
			PyExc_ValueError,
			"%d: Invalid locking command.", cmd
		);
		return NULL;
	}

	if ((fl.l_type != F_RDLCK) &&
	    (fl.l_type != F_WRLCK) &&
	    (fl.l_type != F_UNLCK)) {
		PyErr_Format(
			PyExc_ValueError,
			"%d: Invalid lock type.", fl.l_type
		);
		return NULL;
	}

	if ((cmd == F_GETLK) && (fl.l_type == F_UNLCK)) {
		PyErr_SetString(
			PyExc_ValueError,
			"Lock type of F_UNLCK may not be specified "
			"for an operation to read lock"
		);
		return NULL;
	}

	if (glfs_posix_lock(self->fd, cmd, &fl) != 0) {
		set_glfs_exc("glfs_posix_lock()");
		return NULL;
	}

	if (!verbose)
		Py_RETURN_NONE;

	return Py_BuildValue(
		"{s:i,s:h,s:h,s:L,s:L,s:i}",
		"command", cmd,
		"type", fl.l_type,
		"whence", fl.l_whence,
		"start", fl.l_start,
		"length", fl.l_len,
		"pid", fl.l_pid
	);
}

static PyMethodDef py_glfs_fd_methods[] = {
	{
		.ml_name = "fstat",
		.ml_meth = (PyCFunction)py_glfs_fd_fstat,
		.ml_flags = METH_NOARGS,
		.ml_doc = "fstat gluster fd."
	},
	{
		.ml_name = "fsync",
		.ml_meth = (PyCFunction)py_glfs_fd_fsync,
		.ml_flags = METH_NOARGS,
		.ml_doc = "fstat gluster fd."
	},
	{
		.ml_name = "fchdir",
		.ml_meth = (PyCFunction)py_glfs_fd_fchdir,
		.ml_flags = METH_NOARGS,
		.ml_doc = "change directory to path underlying fd"
	},
	{
		.ml_name = "fchmod",
		.ml_meth = (PyCFunction)py_glfs_fd_fchmod,
		.ml_flags = METH_VARARGS,
		.ml_doc = "change permissions"
	},
	{
		.ml_name = "fchown",
		.ml_meth = (PyCFunction)py_glfs_fd_fchown,
		.ml_flags = METH_VARARGS,
		.ml_doc = "change owner"
	},
	{
		.ml_name = "ftruncate",
		.ml_meth = (PyCFunction)py_glfs_fd_ftruncate,
		.ml_flags = METH_VARARGS,
		.ml_doc = "truncate file"
	},
	{
		.ml_name = "lseek",
		.ml_meth = (PyCFunction)py_glfs_fd_lseek,
		.ml_flags = METH_VARARGS,
		.ml_doc = "set position of fd"
	},
	{
		.ml_name = "pread",
		.ml_meth = (PyCFunction)py_glfs_fd_pread,
		.ml_flags = METH_VARARGS,
		.ml_doc = "read file"
	},
	{
		.ml_name = "pwrite",
		.ml_meth = (PyCFunction)py_glfs_fd_pwrite,
		.ml_flags = METH_VARARGS,
		.ml_doc = "write to file"
	},
	{
		.ml_name = "posix_lock",
		.ml_meth = (PyCFunction)py_glfs_fd_posix_lock,
		.ml_flags = METH_VARARGS | METH_KEYWORDS,
		.ml_doc = "lock file"
	},
	{ NULL, NULL, 0, NULL }
};

static PyGetSetDef py_glfs_fd_getsetters[] = {
	{ .name = NULL }
};

PyTypeObject PyGlfsFd = {
	.tp_name = "pyglfs.FD",
	.tp_basicsize = sizeof(py_glfs_fd_t),
	.tp_methods = py_glfs_fd_methods,
	.tp_getset = py_glfs_fd_getsetters,
	.tp_new = py_glfs_fd_new,
	.tp_init = py_glfs_fd_init,
	.tp_doc = "Glusterfs file handle",
	.tp_dealloc = (destructor)py_glfs_fd_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
};
