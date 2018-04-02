# -*- coding: utf-8 -*-
"""
    3D grid for raytracing based on the fast sweeping method

    This code is part of ttcr ( https://github.com/groupeLIAMG/ttcr )
"""

"""
Copyright 2017 Bernard Giroux
email: bernard.giroux@ete.inrs.ca

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

from libcpp.string cimport string
from libcpp.vector cimport vector
from libc.stdint cimport uint32_t
from libcpp cimport bool

import numpy as np
cimport numpy as np

from scipy.sparse import csr_matrix

cdef extern from "ttcr_t.h" namespace "ttcr":
    cdef cppclass sxyz[T]:
        sxyz(T, T, T) except +
        T x
        T y
        T z

    cdef cppclass siv[T]:
        siv(size_t, T) except +
        size_t i
        T v

    cdef cppclass sijv[T]:
        sijv(size_t, T) except +
        size_t i
        size_t j
        T v

cdef extern from "Grid3Drifs.h" namespace "ttcr":
    cdef cppclass Grid3Drifs[T1,T2]:
        Grid3Drifs(T2, T2, T2, T1, T1, T1, T1, T1, int, bool, size_t) except +
        size_t getNthreads()
        void setSlowness(vector[T1]&) except +
        void raytrace(vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[sxyz[T1]]&,
                      vector[T1]&,
                      size_t) except +
        void raytrace(vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[vector[sxyz[T1]]]&,
                      T1&,
                      size_t) except +
        void raytrace(vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[vector[sxyz[T1]]]&,
                      T1&,
                      vector[vector[sijv[T1]]]&,
                      size_t) except +

cdef extern from "Grid3Drcfs.h" namespace "ttcr":
    cdef cppclass Grid3Drcfs[T1,T2]:
        Grid3Drcfs(T2, T2, T2, T1, T1, T1, T1, T1, int, bool, size_t) except +
        size_t getNthreads()
        void setSlowness(vector[T1]&) except +
        void raytrace(vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[sxyz[T1]]&,
                      vector[T1]&,
                      size_t) except +
        void raytrace(vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[vector[siv[T1]]]&,
                      size_t) except +
        void raytrace(vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[sxyz[T1]]&,
                      vector[T1]&,
                      vector[vector[sxyz[T1]]]&,
                      vector[vector[siv[T1]]]&,
                      size_t) except +

cdef class Grid3Drn:
    """
    Grid3Drn(nx, ny, nz, dx, xmin, ymin, zmin, eps, maxit, weno, nthreads)

    3D rectilinear grid with slowness defined at nodes

    Parameters
    ----------
    nx : number of cells in x
    ny : number of cells in y
    nz : number of cells in z
    dx : cell size in x
    xmin : origin in x
    ymin : origin in y
    zmin : origin in z
    eps : convergence criterion (FSM)
    maxit : max number of sweeping iterations
    weno : use 3rd order weighted essentially non-oscillatory operator (bool)
    nthreads : number of threads for raytracing
    """
    cdef uint32_t nx
    cdef uint32_t ny
    cdef uint32_t nz
    cdef Grid3Drifs[double, uint32_t]* grid
    def __cinit__(self, uint32_t nx, uint32_t ny, uint32_t nz, double dx,
                  double xmin, double ymin, double zmin,
                  double eps, int maxit, bool weno, size_t nthreads):
        self.nx = nx
        self.ny = ny
        self.nz = nz
        self.grid = new Grid3Drifs[double,uint32_t](nx, ny, nz, dx, xmin, ymin,
                                  zmin, eps, maxit, weno, nthreads)


    def __dealloc__(self):
        del self.grid

    def get_nthreads(self):
        """
        Returns
        -------
        number of threads allowed for calculations
        """
        return self.grid.getNthreads()

    def set_slowness(self, slowness):
        """
        Assign slowness at grid nodes

        Parameters
        ----------
        slowness : 1D array (in 'C' order)
        """
        cdef vector[double] slown
        nx = self.nx+1
        ny = self.ny+1
        nz = self.nz+1
        for k in range(nz):
            for j in range(ny):
                for i in range(nx):
                    # slowness is in 'C' order and we must pass it in 'F' order
                    slown.push_back(slowness[(i*ny + j)*nz + k])
        self.grid.setSlowness(slown)

    def raytrace(self, slowness, Tx, Rx, t0=0.0, nout=1, thread_no=0):
        """
        Perform raytracing for a single source

        The source can comprise more than one point

        Parameters
        ----------
            slowness : 1D array od slowness values at nodes (can be None, but
                       slowness should have been set before calling raytrace)
            Tx : coordinates of source points (npts x 3)
            Rx : coordinates of receivers (nrcv x 3)
            t0 : time of source event
            nout : number of parameters to return (see below)
            thread_no : thread/process number on which computation should be run

        Returns
        -------
            tt : travel time are receivers
            if nout == 3 or nout == 4:
                rays : list holding coordinates of ray segments, for each rcv
                v0 : average velocity at source points
            if nout == 4:
                M : matrix of partial derivatives of t w/r to velocity
        """
        # assing model data
        cdef vector[double] slown
        if slowness is not None:
            nx = self.nx+1
            ny = self.ny+1
            nz = self.nz+1
            for k in range(nz):
                for j in range(ny):
                    for i in range(nx):
                        # slowness is in 'C' order and we must pass it in 'F' order
                        slown.push_back(slowness[(i*ny + j)*nz + k])
            self.grid.setSlowness(slown)

        cdef vector[sxyz[double]] vTx
        cdef vector[sxyz[double]] vRx
        cdef vector[double] vt0
        cdef vector[double] vtt

        for t in Tx:
            vTx.push_back(sxyz[double](t[0], t[1], t[2]))
        for r in Rx:
            vRx.push_back(sxyz[double](r[0], r[1], r[2]))
        vt0.push_back(t0)
        vtt.resize(Rx.shape[0])

        cdef vector[vector[sxyz[double]]] r_data
        cdef vector[vector[sijv[double]]] m_data
        cdef double v0

        if nout == 1:
            self.grid.raytrace(vTx, vt0, vRx, vtt, thread_no)

            tt = np.empty((Rx.shape[0],))
            for n in range(Rx.shape[0]):
                tt[n] = vtt[n]

            return tt

        elif nout == 3:
            self.grid.raytrace(vTx, vt0, vRx, vtt, r_data, v0, thread_no)

            rays = [ [0.0] for i in range(Rx.shape[0]) ]
            tt = np.empty((Rx.shape[0],))
            for n in range(Rx.shape[0]):
                tt[n] = vtt[n]
                rays[n] = np.empty((r_data[n].size(), 3))
                for nn in range(r_data[n].size()):
                    rays[n][nn, 0] = r_data[n][nn].x
                    rays[n][nn, 1] = r_data[n][nn].y
                    rays[n][nn, 2] = r_data[n][nn].z

            return tt, rays, v0

        elif nout == 4:
            self.grid.raytrace(vTx, vt0, vRx, vtt, r_data, v0, m_data, thread_no)

            rays = [ [0.0] for i in range(Rx.shape[0]) ]
            tt = np.empty((Rx.shape[0],))
            for n in range(Rx.shape[0]):
                tt[n] = vtt[n]
                rays[n] = np.empty((r_data[n].size(), 3))
                for nn in range(r_data[n].size()):
                    rays[n][nn, 0] = r_data[n][nn].x
                    rays[n][nn, 1] = r_data[n][nn].y
                    rays[n][nn, 2] = r_data[n][nn].z

            nnz = 0
            for ni in range(m_data.size()):
                nnz += m_data[ni].size()
            indptr = np.empty((Rx.shape[0]+1,), dtype=np.int64)
            indices = np.empty((nnz,), dtype=np.int64)
            val = np.empty((nnz,))

            k = 0
            M = Rx.shape[0]
            N = (self.nx+1)*(self.ny+1)*(self.nz+1)
            for i in range(M):
                indptr[i] = k
                for j in range(N):
                    for n in range(m_data[i].size()):
                        if m_data[i][n].i == i and m_data[i][n].j == j:
                            indices[k] = j
                            val[k] = m_data[i][n].v
                            k += 1

            indptr[M] = k
            MM = csr_matrix((val, indices, indptr), shape=(M,N))

            return tt, rays, v0, MM


cdef class Grid3Drc:
    """
    Grid3Drc(nx, ny, nz, dx, xmin, ymin, zmin, eps, maxit, weno, nthreads)

    3D rectilinear grid with slowness defined in cells

    Parameters
    ----------
    nx : number of cells in x
    ny : number of cells in y
    nz : number of cells in z
    dx : cell size in x
    xmin : origin in x
    ymin : origin in y
    zmin : origin in z
    eps : convergence criterion (FSM)
    maxit : max number of sweeping iterations
    weno : use 3rd order weighted essentially non-oscillatory operator (bool)
    nthreads : number of threads for raytracing
    """
    cdef uint32_t nx
    cdef uint32_t ny
    cdef uint32_t nz
    cdef Grid3Drcfs[double, uint32_t]* grid
    def __cinit__(self, uint32_t nx, uint32_t ny, uint32_t nz, double dx,
                  double xmin, double ymin, double zmin,
                  double eps, int maxit, bool weno, size_t nthreads):
        self.nx = nx
        self.ny = ny
        self.nz = nz
        self.grid = new Grid3Drcfs[double,uint32_t](nx, ny, nz, dx, xmin, ymin,
                                  zmin, eps, maxit, weno, nthreads)

    def __dealloc__(self):
        del self.grid

    def get_nthreads(self):
        """
        Returns
        -------
        number of threads allowed for calculations
        """
        return self.grid.getNthreads()

    def set_slowness(self, slowness):
        """
        Assign slowness at grid cells

        Parameters
        ----------
        slowness : 1D array (in 'C' order)
        """
        cdef vector[double] slown
        for k in range(self.nz):
            for j in range(self.ny):
                for i in range(self.nx):
                    # slowness is in 'C' order and we must pass it in 'F' order
                    slown.push_back(slowness[(i*self.ny + j)*self.nz + k])
        self.grid.setSlowness(slown)

    def raytrace(self, slowness, Tx, Rx, t0, nout=1, thread_no=0):
        """
        Perform raytracing for a single source

        The source can comprise more than one point

        Parameters
        ----------
            slowness : 1D array od slowness values in cells (can be None, but
                       slowness should have been set before calling raytrace)
            Tx : coordinates of source points (npts x 3)
            Rx : coordinates of receivers (nrcv x 3)
            t0 : time of source event
            nout : number of parameters to return (see below)
            thread_no : thread/process number on which computation should be run

        Returns
        -------
            tt : travel time are receivers
            if nout == 2 or nout == 3:
                L : ray projection matrix
            if nout == 3:
                rays : list holding coordinates of ray segments, for each rcv
        """
        # assing model data
        cdef vector[double] slown
        if slowness is not None:
            for k in range(self.nz):
                for j in range(self.ny):
                    for i in range(self.nx):
                        # slowness is in 'C' order and we must pass it in 'F' order
                        slown.push_back(slowness[(i*self.ny + j)*self.nz + k])
            self.grid.setSlowness(slown)

        cdef vector[sxyz[double]] vTx
        cdef vector[sxyz[double]] vRx
        cdef vector[double] vt0
        cdef vector[double] vtt

        for t in Tx:
            vTx.push_back(sxyz[double](t[0], t[1], t[2]))
        for r in Rx:
            vRx.push_back(sxyz[double](r[0], r[1], r[2]))
        vt0.push_back(t0)
        vtt.resize(Rx.shape[0])

        cdef vector[vector[siv[double]]] l_data
        cdef vector[vector[sxyz[double]]] r_data
        cdef double v0

        if nout == 1:
            self.grid.raytrace(vTx, vt0, vRx, vtt, thread_no)

            tt = np.empty((Rx.shape[0],))
            for n in range(Rx.shape[0]):
                tt[n] = vtt[n]

            return tt

        elif nout == 2:
            self.grid.raytrace(vTx, vt0, vRx, vtt, l_data, thread_no)

            tt = np.empty((Rx.shape[0],))
            for n in range(Rx.shape[0]):
                tt[n] = vtt[n]

            indptr = np.empty((Rx.shape[0]+1,), dtype=np.int64)
            indices = np.empty((l_data.size(),), dtype=np.int64)
            val = np.empty((l_data.size(),))

            k = 0
            M = Rx.shape[0]
            N = self.nx*self.ny*self.nz
            for i in range(M):
                indptr[i] = k
                for j in range(N):
                    for n in range(l_data[i].size()):
                        if l_data[i][n].i == j:
                            indices[k] = j
                            val[k] = l_data[i][n].v
                            k += 1

            indptr[M] = k
            L = csr_matrix((val, indices, indptr), shape=(M,N))

            return tt, L

        elif nout == 3:
            self.grid.raytrace(vTx, vt0, vRx, vtt, r_data, l_data, thread_no)

            rays = [ [0.0] for i in range(Rx.shape[0]) ]
            tt = np.empty((Rx.shape[0],))
            for n in range(Rx.shape[0]):
                tt[n] = vtt[n]
                rays[n] = np.empty((r_data[n].size(), 3))
                for nn in range(r_data[n].size()):
                    rays[n][nn, 0] = r_data[n][nn].x
                    rays[n][nn, 1] = r_data[n][nn].y
                    rays[n][nn, 2] = r_data[n][nn].z


            indptr = np.empty((Rx.shape[0]+1,), dtype=np.int64)
            indices = np.empty((l_data.size(),), dtype=np.int64)
            val = np.empty((l_data.size(),))

            k = 0
            M = Rx.shape[0]
            N = self.nx*self.ny*self.nz
            for i in range(M):
                indptr[i] = k
                for j in range(N):
                    for n in range(l_data[i].size()):
                        if l_data[i][n].i == j:
                            indices[k] = j
                            val[k] = l_data[i][n].v
                            k += 1

            indptr[M] = k
            L = csr_matrix((val, indices, indptr), shape=(M,N))

            return tt, L, rays