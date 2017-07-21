
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _AFX_RESIDUE_MGR_H_
#define _AFX_RESIDUE_MGR_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxZodiacData;
class afxModel;

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxResidueMgr
//
//    Manage transient objects in the world.

class afxResidueMgr : public GameBase
{

  typedef GameBase Parent;
 
  enum { 
    ZODIAC,
    MODEL
  };

  struct Residue
  {
    struct ZodiacParams
    {
      F32   pos_x, pos_y, pos_z;
      F32   rad, vrange_dn, vrange_up;
      U8    r,g,b,a;
      F32   ang;
      bool  on_terrain;
    };  
    
    union ResidueParams
    {
      ZodiacParams  zodiac;
    };
    
    union ResidueData
    {
      afxZodiacData*  zodiac;
      afxModel*       model;
      SimObject*      simobject;
    };

    
    U32           type;
    ResidueData   data;
    ResidueParams params;
    U32           fade_time;
    U32           stop_time;
    F32           fade;
        
    Residue*      next;
  };

  class ResidueList
  {
    Vector<Residue*>  m_array_a;
    Vector<Residue*>  m_array_b;

    Vector<Residue*>* m_array;
    Vector<Residue*>* m_scratch_array;
    bool              m_dirty;
    S32               m_pending;

    void              swap_array_ptrs();
    void              free_residue(Residue*);

  public:
    /*C*/             ResidueList();
    /*D*/             ~ResidueList();

    void              clear();
    S32               size() { return m_array->size(); }
    bool              empty() { return m_array->empty(); }
    void              sortIfDirty() { if (m_dirty) sort(); }

    void              sort();
    void              fadeAndCull(U32 now);
    void              stripMatchingObjects(SimObject* db, bool del_notify=false);
    void              add(Residue*);

    void              manage();

    U32               findPendingBestBump(U32 look_max=256);
    void              bumpPending();

    static int QSORT_CALLBACK compare_residue(const void* p1, const void* p2);
  };

  friend class ResidueList;

private:
  enum { FREE_POOL_BLOCK_SIZE = 256 };

  static afxResidueMgr* the_mgr;

  static U32        m_max_residue_objs;
  static bool       enabled;

  ResidueList       m_managed;
  
  Vector<Residue*>  m_free_pool_blocks;
  Residue*          m_next_free;

  Residue*          alloc_free_pool_block();
  Residue*          alloc_residue();
  void              free_residue(Residue*);

  void              bump_residue();
  void              add_residue(Residue*);
  static void       add_zodiac(F32 dur, F32 fade_dur, afxZodiacData*, const Point3F& pos, F32 rad, 
                               const Point2F& vrange, const ColorF& col, F32 ang, bool on_terrain);
 
protected:
  void              deleteResidueObject(SimObject* obj, bool del_notify=false); 

  void              manage_residue(const Residue* r);

  bool              requires_delete_tracking(Residue*);
  void              enable_delete_tracking(Residue*);
  void              disable_delete_tracking(Residue*);
   
public:                     
  /*C*/             afxResidueMgr();
  /*D*/             ~afxResidueMgr();

  void              cleanup();
  virtual void      onDeleteNotify(SimObject *obj);
    
public:
  void              residueAdvanceTime();

                    // ZODIAC
  static void       add_terrain_zodiac(F32 dur, F32 fade_dur, afxZodiacData*, const Point3F& pos, F32 rad,
                                       const ColorF& col, F32 ang);
  static void       add_interior_zodiac(F32 dur, F32 fade_dur, afxZodiacData*, const Point3F& pos, F32 rad,
                                        const Point2F& vrange, const ColorF& col, F32 ang);

                    // MODEL
  static void       add(F32 dur, F32 fade_dur, afxModel*);

  static afxResidueMgr* getMaster() { return the_mgr; }
  static void           setMaster(afxResidueMgr* m) { the_mgr = m; }

  DECLARE_CONOBJECT(afxResidueMgr);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_RESIDUE_MGR_H_
