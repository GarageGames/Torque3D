Greed v00.00.08a
-------

Changelog :
-------

###### .01a : 95% of physx3 should be working thanks to rextimmy/andr3wmac. ######
###### .02b : the Greed folder was updated to current physx3 from rextimmy, more to come
###### on physx3.
###### .03b : re-added default readme, started proper changelog.
###### .04b : switched theme of torque3d to the DarkUi.
http://www.garagegames.com/community/resources/view/22590
###### .05a  : alpha : started testing fps performances on d3d9-refactor. (avg fps on default : 103.45)
###### .05b  : alpha : first official commit, refactor was removed : half the fps were lost
###### .06a  : alpha : all physx support dropped, Bullet support 100%, rextimmy core physics changes in
###### GMK is tested and fixed and in process of reintegration. physx3 basic support is provided by T3D.
----------------
###### .07 goals: new car, new avatar, island textured + base scene setup. 
----------------
----------------
###### .08a base code is frozen appart for fixes, integration of .07a goals are partly done (base island is there)
----------------

This is a fork of the MIT Licensed Torque 3D (see http://www.garagegames.com/products/torque-3d).

More Information
----------------
* Torque 3D [GitHub Wiki](https://github.com/GarageGames/Torque3D/wiki)
* Documentation is in the [Torque3D-Documentation](https://github.com/GarageGames/Torque3D-Documentation) GitHub repo.
* Project Manager is in the [Torque3D-ProjectManager](https://github.com/GarageGames/Torque3D-ProjectManager) GitHub repo.

Creating a New Project Based on a Template
------------------------------------------

The templates included with Torque 3D provide a starting point for your project.  
Once we have created our own project based on a template, we may then compile an 
executable and begin work on our game.  
The following templates are included in this version of Torque 3D:

* Empty
* Full
* TheFunBase : contains any Greed specific changes, bullet activated for physics.

### Using Greed ###
As of 0.6, bullet is mapped and can be used like stock physics, there is also
a native rigid physic shape that can be used as well as native GMK shapes.
to note : no native bullet vehicle yet.

### Using the Project Manager to Create a Project ###

The *Project Manager* may be used to create a new game project based on one of the templates that are included with Greed. 
For now base Full and Empty and TheFunBase templates are available, altough, the fps tutorial is probably useable 
with scripts fixs.

Greed License
-------
Greed is MIT Licensed as the original Torque 3D software is.
-------
Hénarès Sébastien aka dragutux.

Original Copyright Torque 3D MIT 3.5 (c) 2012 GarageGames, LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

