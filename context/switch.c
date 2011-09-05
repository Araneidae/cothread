/* This file is part of the Diamond cothread library.
 *
 * Copyright (C) 2011 Michael Abbott, Diamond Light Source Ltd.
 *
 * The Diamond cothread library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * The Diamond cothread library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Contact:
 *      Dr. Michael Abbott,
 *      Diamond Light Source Ltd,
 *      Diamond House,
 *      Chilton,
 *      Didcot,
 *      Oxfordshire,
 *      OX11 0DE
 *      michael.abbott@diamond.ac.uk
 */

#if defined(__APPLE__)
    #define FNAME(name) \
        ".globl _" #name "\n_" #name ":\n"
    #define FSIZE(name)
#else
    #define FNAME(name) \
        ".globl " #name "\n" \
        ".type " #name " STT_FUNC\n" \
        #name ":\n"
    #define FSIZE(name) \
        ".size " #name ", .-" #name "\n"
#endif

#if defined(__i386__)
    #include "switch-x86.c"
#elif defined(__x86_64__)
    #include "switch-x86_64.c"
#elif defined(__arm__)  &&  defined(__unix__)
    #include "switch-arm.c"
#elif defined(__ppc__)  &&  defined(__APPLE__)
    #include "switch-ppc_osx.c"
#else
    #error "Don't know how to support this platform"
#endif

/* For Windows support we'll have a number of problems.  Firstly, what tool are
 * we going to build with?  Think I'd prefer mingw, but we'll have to see.  This
 * may well affect the assembler syntax as well.
 *
 * Secondly, the ABIs differ.  Variable names for Windows follow the OSX
 * convention, so that's ok, and it looks as if the 32-bit x86 ABI is identical,
 * so with luck we may be able to use switch-x86.c unchanged.
 *
 * The 64-bit ABI is somewhat gratuitously incompatible on the other hand.
 *
 * According to:
 *  http://msdn.microsoft.com/en-US/library/ms235286(v=VS.80).aspx
 *
 * The register usage is a bit different from Unix:
 *
 *  rax     Result register
 *  rbx     Must be preserved
 *  rcx     First argument
 *  rdx     Second argument
 *  rsp     Stack pointer, must be preserved
 *  rbp     Must be preserved
 *  rsi     Must be preserved
 *  rdi     Must be preserved
 *  r8      Third argument
 *  r9      Fourth argument
 *  r10-r11 Volatile
 *  r12-r15 Must be preserved
 *  xmm0-5  Volatile
 *  xmm6-15 Must be preserved
 *
 * So we need to preserve:
 *  rbx, rbp, rsi, rdi, r12-r15, xmm6-xmm15.
 *
 * Also fpcsr must be preserved ... but functions should not be called with
 * modified values anyway.  Same rules for mxcsr.
 *
 * So with a little care we ought to be able to target windows with the same
 * source.
 *
 * So let's look at the differences:
 *
 *             Unix        Windows
 * Result      rax         rax         The same
 * P1          rdi         rcx
 * P2          rsi         rdx
 * P3          rdx         r8
 *
 * Saves
 *             rbx,rbp     rbx,rbp     Same
 *             r12-r15     r12-r15     Same
 *                         rsi,rdi     Windows only
 *             fpcsr,mxcsr fpcsr,mxcsr Same
 *                         xmm6-15     Windows only (and bloody huge)
 *
 * Tempting to solve the initialisation problem by zero setting the frame... */
