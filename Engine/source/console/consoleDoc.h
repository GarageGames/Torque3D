//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// This file exists solely to document consoleDoc.cpp

/// @defgroup  script_autodoc Script Auto-Documentation
/// @ingroup   console_system Console System
///
/// @section script_autodoc_using        Using Script Auto-Documentation
///
/// There are on the order of three hundred functions exposed to the script language
/// through the console. It is therefore extremely important that they be documented,
/// but due to their number, it is difficult to maintain a seperate reference document.
/// 
/// Therefore, a simple documentation system has been built into the scripting engine. It
/// was initially added by Mark Frohnmayer, and later enhanced by Ben Garney. The
/// scripting engine supports grouping functions and methods, to help organize the 
/// several hundred functions, as well as associating a "usage string" with functions and
/// groups.
///
/// @note The results of a console doc dump will vary depending on when you run it. If
///       you run it, for example, while in the game menu, it won't output any data for
///       the script-defined classes which are defined for gameplay. To get comprehensive
///       documentation, you may need to write a special script that will get all your
///       classes loaded into the scripting engine.
///
/// The script documentation system is designed to output a dump of the current state
/// of the scripting engine in a format understandable by Doxygen. It does this by 
/// traversing the namespace/class hierarchy in memory at the time of the dump, and
/// outputting psuedo-C++ code equivalent to this class hierarchy.
/// 
/// @subsection script_autodoc_using_script For the Scripter...
///
/// Currently, there is no way to associate usage strings or other documentation with script code
/// like you can with C++ code.
///
/// You can get a list of all the methods and fields of an object from any object which inherits
/// from SimObject (ie, every object), as well as the documentation on those objects by using the
/// dump() method from the console:
///
/// @code
/// ==>$foo = new SimObject();
/// ==>$foo.dump();
/// Member Fields:
/// Tagged Fields:
/// Methods:
///   delete() - obj.delete()
///   dump() - obj.dump()
///   getClassName() - obj.getClassName()
///   getGroup() - obj.getGroup()
///   getId() - obj.getId()
///   getName() - obj.getName()
///   getType() - obj.getType()
///   save() - obj.save(fileName, <selectedOnly>)
///   schedule() - object.schedule(time, command, <arg1...argN>);
///   setName() - obj.setName(newName)
/// @endcode
///
/// In the Torque example app, there are two functions defined in common\\client\\scriptDoc.cs
/// which automate the process of dumping the documentation. They make use of the ConsoleLogger
/// object to output the documentation to a file, and look like this:
///
/// @note You may want to add this code, or code like it, to your project if you have
///       rewritten the script code in common. 
///
/// @code
/// // Writes out all script functions to a file
/// function writeOutFunctions() {
///    new ConsoleLogger( logger, "ConsoleFunctions.txt", false );
///    dumpConsoleFunctions();
///    logger.delete();
/// }
/// 
/// // Writes out all script classes to a file
/// function writeOutClasses() {
///    new ConsoleLogger( logger, "scriptClasses.txt", false );
///    dumpConsoleClasses();
///    logger.delete();
/// }
/// @endcode
///
/// @subsection script_autodoc_using_coder  For the C++ Coder...
///
/// @note <b>It is of the utmost important that you keep your usage strings up to date!</b>
///       Usage strings are the only way that a scripter has to know how to use the methods,
///       functions, and variables you expose. Misleading, missing, or out of date documentation
///       will make their lives much harder - and yours, too, because you'll have to keep
///       explaining things to them! So make everyone's lives easier - keep your usage strings
///       clear, concise, and up to date.
///
/// There are four types of items which can be documented using the autodocumentation system: 
///   - <b>Fields</b>, which are defined using the addField() calls. They are documented
///     by passing a string to the usage parameter.
///   - <b>Field groups</b>, which are defined using the beginGroup() and endGroup() calls.
///     They are documented by passing a descriptive string to the usage parameter.
///   - <b>Method groups</b>, which are defined using beginCommandGroup(), endCommandGroup(),
///     ConsoleMethodGroupEnd(), ConsoleMethodGroupBegin(), ConsoleFunctionGroupEnd(), and
///     ConsoleFunctionGroupBegin().
///   - <b>Methods and functions</b>, which are defined using either SimObject::addCommand(),
///     the ConsoleMethod() macro, or the ConsoleFunction() macro. Methods and functions are
///     special in that the usage strings should be in a specific format, so
///     that parameter information can be extracted from them and placed into the Doxygen
///     output. 
///
/// You can use standard Doxygen commands in your comments, to make the documentation clearer.
/// Of particular use are \@returns, \@param, \@note, and \@deprecated.
///
/// <b>Examples using global definitions.</b>
///
/// @code
///    // Example of using Doxygen commands.
///    ConsoleFunction(alxGetWaveLen, S32, 2, 2, "(string filename)"
///                    "Get length of a wave file\n\n"
///                    "@param filename File to determine length of.\n"
///                    "@returns Length in milliseconds.")
///
///    // A function group...
///    ConsoleFunctionGroupBegin(Example, "This is an example group! Notice that the name for the group"
///                                       "must be a valid identifier, due to limitations in the C preprocessor.");
///
///    // ConsoleFunction definitions go here.
///
///    ConsoleFunctionGroupEnd(Example);
///
///    // You can do similar things with methods...
///    ConsoleMethodGroupBegin(SimSet, UsefulFuncs, "Here are some useful functions involving a SimSet.");
///    ConsoleMethod(SimSet, listObjects, void, 2, 2, "set.listObjects();")
///    ConsoleMethodGroupEnd(SimSet, UsefulFuncs, "Here are some more useful functions involving a SimSet.");
/// @endcode
///
/// <b>Examples using addField</b>
///
/// @note Using addCommand is strongly deprecated.
///
/// @code
///   // Example of a field group.
///   addGroup( "Logging", "Things relating to logging." );
///   addField( "level",   TYPEID< ConsoleLogLevel >(),     Offset( mLevel,    ConsoleLogger ) );
///   endGroup( "Logging" );
/// @endcode
///
/// @section script_autodoc_makingdocs   How to Generate Script Docs
///
/// Script docs can be generated by running the dumpConsoleFunctions() and
/// dumpConsoleClasses(), then running the output through Doxygen. There is an
/// example Doxygen configuration file to do this at
/// doc\\doxygen\\html\\script_doxygen.html.cfg. Doxygen will parse the psuedo-C++
/// generated by the console doc code and produce a class hierarchy and documentation
/// of the global namespace. You may need to tweak the paths in the configuration file
/// slightly to reflect your individual setup.
///
/// @section script_autodoc_internals Script Auto-Documentation Internals
/// 
/// The consoleDoc system works by inserting "hidden" entries in Namespace and
/// AbstractClassRep; these hidden entries are assigned special type IDs so that
/// they aren't touched by the standard name resolution code. At documentation
/// creation time, the dumpConsole functions iterate through the Namespace hierarchy
/// and the AbstractClassRep data and extract this "hidden" information, outputting
/// it in a Doxygen-compatible format.
///
/// @note You can customize the output of the console documentation system by modifying
///       these functions:
///            - printClassHeader()
///            - printClassMethod()
///            - printGroupStart()
///            - printClassMember()
///            - printGroupEnd()
///            - printClassFooter()
///
/// @note There was once support for 'overloaded' script functions; ie, script functions
///       with multiple usage strings. Certain functions in the audio library used this.
///       However, it was deemed too complex, and removed from the scripting engine. There
///       are still some latent traces of it, however.
