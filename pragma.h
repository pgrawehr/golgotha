
//                           (clip here)
//
// Pragmas for use with Microsoft Visual C++ 5.0 and 6.0
//
// See http://oakroadsystems.com/tech/msvc.htm for full discussion.
// Briefly, we need to compile at level 4 (/W4) because many important
// warnings, and even some errors, have been assigned the low level of
// 4. But level 4 also contains many spurious warnings, so we want to
// compile at /W2 or /W3. The pragmas in this file promote many errors
// and warnings to level 2 or 3.
//
// This file is published at http://oakroadsystems.com/tech/warnings.htm
// and it was last modified on 12 Jun 2002.
//       Copyright 1998-2002 by Stan Brown, Oak Road Systems
//                    http://oakroadsystems.com/
// License is hereby granted to use this file (or a modified version
// of it) in any source code without payment of any license fee,
// provided this paragraph is retained in its entirety.
//
// If you find this file useful, I'd appreciate your letting me know
// at the above site. If you find any errors, or have improvements to
// suggest, those will be gratefully received and acknowledged.

#pragma warning(2:4032)     // function arg has different type from declaration
#pragma warning(2:4092)     // 'sizeof' value too big
#pragma warning(2:4132 4268)// const object not initialized
#pragma warning(2:4152)     // pointer conversion between function and data
#pragma warning(2:4239)     // standard doesn't allow this conversion
#pragma warning(2:4701)     // local variable used without being initialized
#pragma warning(2:4706)     // if (a=b) instead of (if a==b)
#pragma warning(2:4709)     // comma in array subscript
#pragma warning(4:4061)     // not all enum values tested in switch statement
#pragma warning(4:4710)     // inline function was not inlined
#pragma warning(3:4121)     // space added for structure alignment
#pragma warning(3:4505)     // unreferenced local function removed
#pragma warning(3:4019)     // empty statement at global scope
#pragma warning(3:4057)     // pointers refer to different base types
#pragma warning(3:4125)     // decimal digit terminates octal escape
#pragma warning(2:4131)     // old-style function declarator
#pragma warning(3:4211)     // extern redefined as static
#pragma warning(3:4213)     // cast on left side of = is non-standard
#pragma warning(3:4222)     // member function at file scope shouldn't be static
#pragma warning(3:4234 4235)// keyword not supported or reserved for future
#pragma warning(3:4504)     // type ambiguous; simplify code
#pragma warning(3:4507)     // explicit linkage specified after default linkage
#pragma warning(3:4515)     // namespace uses itself
#pragma warning(3:4516 4517)// access declarations are deprecated
#pragma warning(3:4670)     // base class of thrown object is inaccessible
#pragma warning(3:4671)     // copy ctor of thrown object is inaccessible
#pragma warning(3:4673)     // thrown object cannot be handled in catch block
#pragma warning(3:4674)     // dtor of thrown object is inaccessible
#pragma warning(3:4705)     // statement has no effect (example: a+1;)
#pragma warning(error: 4035)// function does not return a value.
#pragma warning(3:4127)     // expression is constant
#pragma warning(error: 4034) // sizeof returns 0
#pragma warning(3:4130)     // Comparison of string using ==
#pragma warning(error: 4131) //Old style function declaration
#pragma warning(2: 4263 4264 4301) //Virtual function has similar name than base class, but is not overriding. 
#pragma warning(1: 4249) // A member of the virtual base class cannot be accessed from the derived class because the derived class does not have access rights. The member was not accessed.
//#pragma warning(3:4189)     // initialized but not used local var
//
//                           (clip here)





//contact info  |  site map 
//this page:  http://oakroadsystems.com/tech/warnings.htm
//Was this page useful? Visit my other Technical Articles.


