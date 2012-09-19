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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include "os/osFile.h"
#include "os/osPrint.h"
#include "util/utilStr.h"
#include "util/utilString.h"
#include "util/utilArray.h"
#include "util/utilMap.h"
#include "util/utilSort.h"

using namespace Torque;

#include <stdio.h>
#include <dirent.h>

//-----------------------------------------------------------------------------

const char* OrderTable[] =
{
   // Extension list as of 8.25.2005
   // http://oss.sgi.com/projects/ogl-sample/registry/

   // Core
   "GL_VERSION_1_1",
   "GL_VERSION_1_2",
   "GL_VERSION_1_3",
   "GL_VERSION_1_4",
   "GL_VERSION_1_5",

   // X Windows
   "GLX_VERSION_1_1",
   "GLX_VERSION_1_2",
   "GLX_VERSION_1_3",
   "GLX_VERSION_1_4",
   "GLX_VERSION_1_5",

   // Arb extension
   "GL_ARB_multitexture",
   "GLX_ARB_get_proc_address",
   "GL_ARB_transpose_matrix",
   "WGL_ARB_buffer_region",
   "GL_ARB_multisample",
   "GL_ARB_texture_env_add",
   "GL_ARB_texture_cube_map",
   "WGL_ARB_extensions_string",
   "WGL_ARB_pixel_format",
   "WGL_ARB_make_current_read",
   "WGL_ARB_pbuffer",
   "GL_ARB_texture_compression",
   "GL_ARB_texture_border_clamp",
   "GL_ARB_point_parameters",
   "GL_ARB_vertex_blend",
   "GL_ARB_matrix_palette",
   "GL_ARB_texture_env_combine",
   "GL_ARB_texture_env_crossbar",
   "GL_ARB_texture_env_dot3",
   "WGL_ARB_render_texture",
   "GL_ARB_texture_mirrored_repeat",
   "GL_ARB_depth_texture",
   "GL_ARB_shadow",
   "GL_ARB_shadow_ambient",
   "GL_ARB_window_pos",
   "GL_ARB_vertex_program",
   "GL_ARB_fragment_program",
   "GL_ARB_vertex_buffer_object",
   "GL_ARB_occlusion_query",
   "GL_ARB_shader_objects",
   "GL_ARB_vertex_shader",
   "GL_ARB_fragment_shader",
   "GL_ARB_shading_language_100",
   "GL_ARB_texture_non_power_of_two",
   "GL_ARB_point_sprite",
   "GL_ARB_fragment_program_shadow",
   "GL_ARB_draw_buffers",
   "GL_ARB_texture_rectangle",
   "GL_ARB_color_buffer_float",
   "GL_ARB_half_float_pixel",
   "GL_ARB_texture_float",
   "GL_ARB_pixel_buffer_object",

   // Misc extensions
   "GL_EXT_abgr",
   "GL_EXT_blend_color",
   "GL_EXT_polygon_offset",
   "GL_EXT_texture",
   "GL_EXT_texture3D",
   "GL_SGIS_texture_filter4",
   "GL_EXT_subtexture",
   "GL_EXT_copy_texture",
   "GL_EXT_histogram",
   "GL_EXT_convolution",
   "GL_SGI_color_matrix",
   "GL_SGI_color_table",
   "GL_SGIS_pixel_texture",
   "GL_SGIX_pixel_texture",
   "GL_SGIS_texture4D",
   "GL_SGI_texture_color_table",
   "GL_EXT_cmyka",
   "GL_EXT_texture_object",
   "GL_SGIS_detail_texture",
   "GL_SGIS_sharpen_texture",
   "GL_EXT_packed_pixels",
   "GL_SGIS_texture_lod",
   "GL_SGIS_multisample",
   "GL_EXT_rescale_normal",
   "GLX_EXT_visual_info",
   "GL_EXT_vertex_array",
   "GL_EXT_misc_attribute",
   "GL_SGIS_generate_mipmap",
   "GL_SGIX_clipmap",
   "GL_SGIX_shadow",
   "GL_SGIS_texture_edge_clamp",
   "GL_SGIS_texture_border_clamp",
   "GL_EXT_blend_minmax",
   "GL_EXT_blend_subtract",
   "GL_EXT_blend_logic_op",
   "GLX_SGI_swap_control",
   "GLX_SGI_video_sync",
   "GLX_SGI_make_current_read",
   "GLX_SGIX_video_source",
   "GLX_EXT_visual_rating",
   "GL_SGIX_interlace",
   "GLX_EXT_import_context",
   "GLX_SGIX_fbconfig",
   "GLX_SGIX_pbuffer",
   "GL_SGIS_texture_select",
   "GL_SGIX_sprite",
   "GL_SGIX_texture_multi_buffer",
   "GL_EXT_point_parameters",
   "GL_SGIX_instruments",
   "GL_SGIX_texture_scale_bias",
   "GL_SGIX_framezoom",
   "GL_SGIX_tag_sample_buffer",
   "GL_SGIX_reference_plane",
   "GL_SGIX_flush_raster",
   "GLX_SGI_cushion",
   "GL_SGIX_depth_texture",
   "GL_SGIS_fog_function",
   "GL_SGIX_fog_offset",
   "GL_HP_image_transform",
   "GL_HP_convolution_border_modes",
   "GL_SGIX_texture_add_env",
   "GL_EXT_color_subtable",
   "GLU_EXT_object_space_tess",
   "GL_PGI_vertex_hints",
   "GL_PGI_misc_hints",
   "GL_EXT_paletted_texture",
   "GL_EXT_clip_volume_hint",
   "GL_SGIX_list_priority",
   "GL_SGIX_ir_instrument1",
   "GLX_SGIX_video_resize",
   "GL_SGIX_texture_lod_bias",
   "GLU_SGI_filter4_parameters",
   "GLX_SGIX_dm_buffer",
   "GL_SGIX_shadow_ambient",
   "GLX_SGIX_swap_group",
   "GLX_SGIX_swap_barrier",
   "GL_EXT_index_texture",
   "GL_EXT_index_material",
   "GL_EXT_index_func",
   "GL_EXT_index_array_formats",
   "GL_EXT_compiled_vertex_array",
   "GL_EXT_cull_vertex",
   "GLU_EXT_nurbs_tessellator",
   "GL_SGIX_ycrcb",
   "GL_EXT_fragment_lighting",
   "GL_IBM_rasterpos_clip",
   "GL_HP_texture_lighting",
   "GL_EXT_draw_range_elements",
   "GL_WIN_phong_shading",
   "GL_WIN_specular_fog",
   "GLX_SGIS_color_range",
   "GL_EXT_light_texture",
   "GL_SGIX_blend_alpha_minmax",
   "GL_EXT_scene_marker",
   "GL_SGIX_pixel_texture_bits",
   "GL_EXT_bgra",
   "GL_SGIX_async",
   "GL_SGIX_async_pixel",
   "GL_SGIX_async_histogram",
   "GL_INTEL_texture_scissor",
   "GL_INTEL_parallel_arrays",
   "GL_HP_occlusion_test",
   "GL_EXT_pixel_transform",
   "GL_EXT_pixel_transform_color_table",
   "GL_EXT_shared_texture_palette",
   "GLX_SGIS_blended_overlay",
   "GL_EXT_separate_specular_color",
   "GL_EXT_secondary_color",
   "GL_EXT_texture_env",
   "GL_EXT_texture_perturb_normal",
   "GL_EXT_multi_draw_arrays",
   "GL_EXT_fog_coord",
   "GL_REND_screen_coordinates",
   "GL_EXT_coordinate_frame",
   "GL_EXT_texture_env_combine",
   "GL_APPLE_specular_vector",
   "GL_APPLE_transform_hint",
   "GL_SUNX_constant_data",
   "GL_SUN_global_alpha",
   "GL_SUN_triangle_list",
   "GL_SUN_vertex",
   "WGL_EXT_display_color_table",
   "WGL_EXT_extensions_string",
   "WGL_EXT_make_current_read",
   "WGL_EXT_pixel_format",
   "WGL_EXT_pbuffer",
   "WGL_EXT_swap_control",
   "GL_EXT_blend_func_separate",
   "GL_INGR_color_clamp",
   "GL_INGR_interlace_read",
   "GL_EXT_stencil_wrap",
   "WGL_EXT_depth_float",
   "GL_EXT_422_pixels",
   "GL_NV_texgen_reflection",
   "GL_SGIX_texture_range",
   "GL_SUN_convolution_border_modes",
   "GLX_SUN_get_transparent_index",
   "GL_EXT_texture_env_add",
   "GL_EXT_texture_lod_bias",
   "GL_EXT_texture_filter_anisotropic",
   "GL_EXT_vertex_weighting",
   "GL_NV_light_max_exponent",
   "GL_NV_vertex_array_range",
   "GL_NV_register_combiners",
   "GL_NV_fog_distance",
   "GL_NV_texgen_emboss",
   "GL_NV_blend_square",
   "GL_NV_texture_env_combine4",
   "GL_MESA_resize_buffers",
   "GL_MESA_window_pos",
   "GL_EXT_texture_compression_s3tc",
   "GL_IBM_cull_vertex",
   "GL_IBM_multimode_draw_arrays",
   "GL_IBM_vertex_array_lists",
   "GL_3DFX_texture_compression_FXT1",
   "GL_3DFX_multisample",
   "GL_3DFX_tbuffer",
   "WGL_EXT_multisample",
   "GL_SGIX_vertex_preclip",
   "GL_SGIX_resample",
   "GL_SGIS_texture_color_mask",
   "GLX_MESA_copy_sub_buffer",
   "GLX_MESA_pixmap_colormap",
   "GLX_MESA_release_buffers",
   "GLX_MESA_set_3dfx_mode",
   "GL_EXT_texture_env_dot3",
   "GL_ATI_texture_mirror_once",
   "GL_NV_fence",
   "GL_IBM_static_data",
   "GL_IBM_texture_mirrored_repeat",
   "GL_NV_evaluators",
   "GL_NV_packed_depth_stencil",
   "GL_NV_register_combiners2",
   "GL_NV_texture_compression_vtc",
   "GL_NV_texture_rectangle",
   "GL_NV_texture_shader",
   "GL_NV_texture_shader2",
   "GL_NV_vertex_array_range2",
   "GL_NV_vertex_program",
   "GLX_SGIX_visual_select_group",
   "GL_SGIX_texture_coordinate_clamp",
   "GLX_OML_swap_method",
   "GLX_OML_sync_control",
   "GL_OML_interlace",
   "GL_OML_subsample",
   "GL_OML_resample",
   "WGL_OML_sync_control",
   "GL_NV_copy_depth_to_color",
   "GL_ATI_envmap_bumpmap",
   "GL_ATI_fragment_shader",
   "GL_ATI_pn_triangles",
   "GL_ATI_vertex_array_object",
   "GL_EXT_vertex_shader",
   "GL_ATI_vertex_streams",
   "WGL_I3D_digital_video_control",
   "WGL_I3D_gamma",
   "WGL_I3D_genlock",
   "WGL_I3D_image_buffer",
   "WGL_I3D_swap_frame_lock",
   "WGL_I3D_swap_frame_usage",
   "GL_ATI_element_array",
   "GL_SUN_mesh_array",
   "GL_SUN_slice_accum",
   "GL_NV_multisample_filter_hint",
   "GL_NV_depth_clamp",
   "GL_NV_occlusion_query",
   "GL_NV_point_sprite",
   "WGL_NV_render_depth_texture",
   "WGL_NV_render_texture_rectangle",
   "GL_NV_texture_shader3",
   "GL_NV_vertex_program1_1",
   "GL_EXT_shadow_funcs",
   "GL_EXT_stencil_two_side",
   "GL_ATI_text_fragment_shader",
   "GL_APPLE_client_storage",
   "GL_APPLE_element_array",
   "GL_APPLE_fence",
   "GL_APPLE_vertex_array_object",
   "GL_APPLE_vertex_array_range",
   "GL_APPLE_ycbcr_422",
   "GL_S3_s3tc",
   "GL_ATI_draw_buffers",
   "WGL_ATI_pixel_format_float",
   "GL_ATI_texture_env_combine3",
   "GL_ATI_texture_float",
   "GL_NV_float_buffer",
   "GL_NV_fragment_program",
   "GL_NV_half_float",
   "GL_NV_pixel_data_range",
   "GL_NV_primitive_restart",
   "GL_NV_texture_expand_normal",
   "GL_NV_vertex_program2",
   "GL_ATI_map_object_buffer",
   "GL_ATI_separate_stencil",
   "GL_ATI_vertex_attrib_array_object",
   "GL_OES_byte_coordinates",
   "GL_OES_fixed_point",
   "GL_OES_single_precision",
   "GL_OES_compressed_paletted_texture",
   "GL_OES_read_format",
   "GL_OES_query_matrix",
   "GL_EXT_depth_bounds_test",
   "GL_EXT_texture_mirror_clamp",
   "GL_EXT_blend_equation_separate",
   "GL_MESA_pack_invert",
   "GL_MESA_ycbcr_texture",
   "GL_EXT_pixel_buffer_object",
   "GL_NV_fragment_program_option",
   "GL_NV_fragment_program2",
   "GL_NV_vertex_program2_option",
   "GL_NV_vertex_program3",
   "GLX_SGIX_hyperpipe",
   "GLX_MESA_agp_offset",
   "GL_EXT_texture_compression_dxt1",
   "GL_EXT_framebuffer_object",
   "GL_GREMEDY_string_marker",
};

Map<String,int> OutputOrder;


//-----------------------------------------------------------------------------

String trim(const char* str)
{
   String ts = "";
   if (str) {
      int s = 0, e = strLength(str);
      for (; s != e; s++) {
         C8 c = str[s];
         if (c != ' ' && c != '\t' && c != '\n')
            break;
      }
      while (s != e) {
         C8 c = str[--e];
         if (c != ' ' && c != '\t' && c != '\n')
            break;
      }
      if (s != e)
         ts.insert(0,str + s,e-s+1);
   }
   return ts;
}


//-----------------------------------------------------------------------------

struct Group
{
   String name;
   String link;
   Array<String> defines;
   Array<String> functions;
   Array<String> types;
   int order;
};
typedef Array<Group> GroupList;

static inline int weight(const String& str)
{
   if (str.find("GL_VERSION") != String::NPos)
      return 0;

   String prefix = str.substr(0,3);
   if (prefix == "GL_")
      return 1;
   if (prefix == "GLU")
      return 2;
   if (prefix == "GLX")
      return 3;
   if (prefix == "WGL")
      return 4;
   return 5;
}

bool operator<(const Group& a,const Group& b)
{
   int wa = weight(a.name);
   int wb = weight(b.name);
   return (wa == wb)? a.name < b.name: wa < wb;
   // return a.order < b.order;
}

bool loadFile(Group& group,String name)
{
   FILE* file = fopen(name.c_str(),"r");
   if (!file) {
      Print("Could not open file " + name);
      return false;
   }

   char buf[512];
   group.name = trim(fgets(buf,sizeof(buf),file));

   Map<String,int>::Iterator entry = OutputOrder.find(group.name);
   if (entry == OutputOrder.end()) {
      Print ("[" + group.name + "]");
   }
   group.order = (entry != OutputOrder.end())? entry->value: 2000;

   while (!feof(file)) {
      String str = trim(fgets(buf,sizeof(buf),file));
      if (!str.length())
         continue;

      if (str.find("http:") != String::NPos) {
         group.link = trim(str);
         continue;
      }
      if (str.find("typedef") != String::NPos) {
         group.types.pushBack(str);
         continue;
      }
      if (str.find("DECLARE") != String::NPos) {
         group.types.pushBack(str);
         continue;
      }
      if (str.find("(") != String::NPos) {
         group.functions.pushBack(str);
         continue;
      }
      group.defines.pushBack(str);
   }
   fclose( file );
   return true;
}

void loadDir(GroupList& groups,String name,const char* filter)
{
   DIR *dir = opendir(name);
   if (!dir) {
      Print("Could not open file " + name);
      return;
   }

   struct dirent *fEntry;
   while ((fEntry = readdir(dir)) != 0) {
      if (fEntry->d_name[0] == '.')
         continue;
      String file = name + "/" + String(fEntry->d_name);
      if (filter[0] != '*' && file.find(filter) == String::NPos)
         continue;
      //Print("Loading " + file);
      groups.pushBack(Group());
      if (!loadFile(groups.last(),file))
         groups.popBack();
   }

   quickSort(groups.begin(),groups.end());
}


//-----------------------------------------------------------------------------

void write(FILE* file,String line)
{
   fwrite(line,1,line.length(),file);
}

void outputHeader(GroupList& groups, String name)
{
   FILE* file = fopen(name.c_str(),"w");
   if (!file)
      Print("Could not open file " + name);

   // Output all the group name together at the top
   for (GroupList::Iterator grp = groups.begin(); grp != groups.end(); grp++)
         write(file,"#define " + grp->name + "\n");

#if 0 // Types now with the group
   // Output all the types for all the extensions
   for (GroupList::Iterator grp = groups.begin(); grp != groups.end(); grp++)
      for (Array<String>::Iterator itr = grp->types.begin();
            itr != grp->types.end(); itr++)
         write(file,*itr + ";\n");
#endif

   // Output the defines for each group
   for (GroupList::Iterator grp = groups.begin(); grp != groups.end(); grp++) {
      if (!grp->name)
         continue;
      write(file,"\n#ifdef " + grp->name + "\n");
      for (Array<String>::Iterator itr = grp->types.begin();
            itr != grp->types.end(); itr++)
         write(file,*itr + ";\n");
      for (Array<String>::Iterator itr = grp->defines.begin();
            itr != grp->defines.end(); itr++) {
         write(file,"#define " + *itr  + "\n");
      }
      for (Array<String>::Iterator itr = grp->functions.begin();
            itr != grp->functions.end(); itr++) {
         String& str = *itr;

         // Parse function "return name (args)".  Start at the back because
         // args is enclosed in (), the name has no spaces, and the return type
         // can be several tokens, such as "void *" or "const char *"
         int b = str.length();
         int a = b - 1;
         while (str[a] != '(')
            a--;
         while (str[--a] == ' ')
            ;
         b = a;
         while (str[a] != ' ')
            a--;
         String name = str.substr(a+1,b - a);

         //
         write(file,"#define "+name+" XGL_FUNCPTR("+name+")\n");
      }
      write(file,"#endif\n");
   }
   fclose(file);
}

void outputFunctions(GroupList& groups, String name)
{
   FILE* file = fopen(name.c_str(),"w");
   if (!file)
      Print("Could not open file " + name);

   // Output the functions for each group
   for (GroupList::Iterator grp = groups.begin(); grp != groups.end(); grp++) {
      if (!grp->name)
         continue;
      if (grp->name == "GL_ARB_imaging")
         // Imaging is include as part of 1.4...
         write(file,"\n#if defined(GL_ARB_imaging) && !defined(GL_VERSION_1_4)\n");
      else
         write(file,"\n#ifdef " + grp->name + "\n");
      write(file,"GL_GROUP_BEGIN(" + grp->name + ")\n");
      for (Array<String>::Iterator itr = grp->functions.begin();
            itr != grp->functions.end(); itr++) {
         String& str = *itr;

         // Parse function "return name (args)".  Start at the back because
         // args is enclosed in (), the name has no spaces, and the return type
         // can be several tokens, such as "void *" or "const char *"
         int b = str.length();
         int a = b - 1;
         while (str[a] != '(')
            a--;
         String args = str.substr(a,b - a);

         while (str[--a] == ' ')
            ;
         b = a;
         while (str[a] != ' ')
            a--;
         String name = str.substr(a+1,b - a);

         while (str[a] == ' ')
            a--;
         String rtype = str.substr(0,a+1);
         //
         write(file,"GL_FUNCTION("+name+","+rtype+","+args+")\n");
      }
      write(file,"GL_GROUP_END()\n#endif\n");
   }
   fclose(file);
}


//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
   Kernel::installDefaultPrint();

   // Build a name -> order map for faster lookups.
   for (int i = 0; i < sizeof(OrderTable) / sizeof(const char*); i++)
      OutputOrder.insert(String(OrderTable[i]),i+1);

   GroupList extensions;
   extensions.reserve(800);
   loadDir(extensions,"core","VERSION");
   loadDir(extensions,"extensions","*");
   outputHeader(extensions,"glext.h");
   outputFunctions(extensions,"glfnext.h");

   return 0;
}

