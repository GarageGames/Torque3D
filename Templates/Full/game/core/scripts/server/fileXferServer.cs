function StartFileTransferServer()
{
   %fts = new netFileServer(FileXferServer);
   %fts.LoadPath("art/decals/*.*", true, false, true);
   %fts.LoadPath("art/environment/*.*", true, false, true);
   %fts.LoadPath("art/forest/*.*", true, false, true);
   %fts.LoadPath("art/gui/*.*", true, false, true);   
   %fts.LoadPath("art/lights/*.*", true, false, true);
   %fts.LoadPath("art/particles/*.*", true, false, true);
   %fts.LoadPath("art/ribbons/*.*", true, false, true);
   %fts.LoadPath("art/roads/*.*", true, false, true);
   %fts.LoadPath("art/shapes/*.*", true, false, true);
   %fts.LoadPath("art/skies/*.*", true, false, true);
   %fts.LoadPath("art/sound/*.*", true, false, true);
   %fts.LoadPath("art/terrains/*.*", true, false, true);
   %fts.LoadPath("art/water/*.*", true, false, true);
   %fts.LoadPath("levels/*.*", true, false, true);
   %fts.start();
}