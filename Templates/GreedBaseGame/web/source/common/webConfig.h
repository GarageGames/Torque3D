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

// Define secure TorqueScript calls by adding them here.
// This list defines the calls that can be made by JavaScript into your game, so be very careful.

const char* gSecureScript[] = { 
    "echo",
    "testJavaScriptBridge",
    //"MyNamespace::myfunction",
    0 //SENTINEL
};

// Define the domains which are allowed to run your game via the web
// Your game plugin will refuse to run from any other domain for security reasons.
// You can turn this off during development and/or for debug builds

//#define WEBDEPLOY_DOMAIN_CHECK 
//#define WEBDEPLOY_DOMAIN_ALLOW_DEBUG

const char* gAllowedDomains[] = { 
    //"www.mydomain.com",
    //"games.myotherdomain.com",
    0 //SENTINEL
};