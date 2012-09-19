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

#ifndef _CREATOR_H_
#define _CREATOR_H_

#ifndef _GUIARRAYCTRL_H_
#include "gui/core/guiArrayCtrl.h"
#endif


/// Creator tree from old editor.  Not used in current editor.
class CreatorTree : public GuiArrayCtrl
{
   typedef GuiArrayCtrl Parent;
   public:

      class Node
      {
         public:
            Node();
            ~Node();

            enum {
               Group       = BIT(0),
               Expanded    = BIT(1),
               Selected    = BIT(2),
               Root        = BIT(3)
            };

            BitSet32             mFlags;
            S32                  mId;
            U32                  mTab;
            Node *               mParent;
            Vector<Node*>        mChildren;
            StringTableEntry     mName;
            StringTableEntry     mValue;

            void expand(bool exp);
            void select(bool sel){mFlags.set(Selected, sel);}

            Node * find(S32 id);

            //
            bool isGroup(){return(mFlags.test(Group));}
            bool isExpanded(){return(mFlags.test(Expanded));}
            bool isSelected(){return(mFlags.test(Selected));}
            bool isRoot(){return(mFlags.test(Root));}
            S32 getId(){return(mId);}
            bool hasChildItem();
            S32 getSelected();

            //
            bool isFirst();
            bool isLast();
      };

      CreatorTree();
      ~CreatorTree();

      //
      S32                        mCurId;
      Node *                     mRoot;
      Vector<Node*>              mNodeList;

      //
      void buildNode(Node * node, U32 tab);
      void build();

      //
      bool addNode(Node * parent, Node * node);
      Node * createNode(const char * name, const char * value, bool group = false, Node * parent = 0);
      Node * findNode(S32 id);
      S32 getSelected(){return(mRoot->getSelected());}

      //
      void expandNode(Node * node, bool expand);
      void selectNode(Node * node, bool select);

      //
      void sort();
      void clear();

      S32                           mTabSize;
      S32                           mMaxWidth;
      S32                           mTxtOffset;

      // GuiControl
      void onMouseDown(const GuiEvent & event);
      void onMouseDragged(const GuiEvent & event);
      void onMouseUp(const GuiEvent & event);
      bool onWake();

      // GuiArrayCtrl
      void onRenderCell(Point2I offset, Point2I cell, bool, bool);

      DECLARE_CONOBJECT(CreatorTree);
      DECLARE_CATEGORY( "Gui Editor" );
};

#endif
