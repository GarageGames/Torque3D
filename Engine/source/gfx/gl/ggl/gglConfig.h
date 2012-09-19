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
// Undefine extensions not required

#define GL_VERSION_1_2
#define GL_VERSION_1_3
#define GL_VERSION_1_4
#define GL_VERSION_1_5
#define GL_VERSION_2_0

#define GL_3DFX_multisample
#define GL_3DFX_tbuffer
#define GL_3DFX_texture_compression_FXT1

#define GL_APPLE_client_storage
#define GL_APPLE_element_array
#define GL_APPLE_fence
#define GL_APPLE_specular_vector
#define GL_APPLE_transform_hint
#define GL_APPLE_vertex_array_object
#define GL_APPLE_vertex_array_range
#define GL_APPLE_ycbcr_422

#define GL_ARB_depth_texture
#define GL_ARB_fragment_program
#define GL_ARB_fragment_shader
#define GL_ARB_imaging
#define GL_ARB_matrix_palette
#define GL_ARB_multisample
#define GL_ARB_multitexture
#define GL_ARB_occlusion_query
#define GL_ARB_pixel_buffer_object
#define GL_ARB_point_parameters
#define GL_ARB_point_sprite
#define GL_ARB_shader_objects
#define GL_ARB_shading_language_100
#define GL_ARB_shadow
#define GL_ARB_shadow_ambient
#define GL_ARB_texture_border_clamp
#define GL_ARB_texture_compression
#define GL_ARB_texture_cube_map
#define GL_ARB_texture_env_add
#define GL_ARB_texture_env_combine
#define GL_ARB_texture_env_crossbar
#define GL_ARB_texture_env_dot3
#define GL_ARB_texture_mirrored_repeat
#define GL_ARB_texture_non_power_of_two
#define GL_ARB_texture_rectangle
#define GL_ARB_transpose_matrix
#define GL_ARB_vertex_blend
#define GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_program
#define GL_ARB_vertex_shader
#define GL_ARB_window_pos

#define GL_ATIX_point_sprites
#define GL_ATIX_texture_env_combine3
#define GL_ATIX_texture_env_route
#define GL_ATIX_vertex_shader_output_point_size

#define GL_ATI_draw_buffers
#define GL_ATI_element_array
#define GL_ATI_envmap_bumpmap
#define GL_ATI_fragment_shader
#define GL_ATI_map_object_buffer
#define GL_ATI_pn_triangles
#define GL_ATI_separate_stencil
#define GL_ATI_text_fragment_shader
#define GL_ATI_texture_env_combine3
#define GL_ATI_texture_float
#define GL_ATI_texture_mirror_once
#define GL_ATI_vertex_array_object
#define GL_ATI_vertex_attrib_array_object
#define GL_ATI_vertex_streams

#define GL_EXT_422_pixels
#define GL_EXT_Cg_shader
#define GL_EXT_abgr
#define GL_EXT_bgra
#define GL_EXT_blend_color
#define GL_EXT_blend_equation_separate
#define GL_EXT_blend_func_separate
#define GL_EXT_blend_logic_op
#define GL_EXT_blend_minmax
#define GL_EXT_blend_subtract
#define GL_EXT_clip_volume_hint
#define GL_EXT_cmyka
#define GL_EXT_color_subtable
#define GL_EXT_compiled_vertex_array
#define GL_EXT_convolution
#define GL_EXT_coordinate_frame
#define GL_EXT_copy_texture
#define GL_EXT_cull_vertex
#define GL_EXT_depth_bounds_test
#define GL_EXT_draw_range_elements
#define GL_EXT_fog_coord
#define GL_EXT_fragment_lighting
#define GL_EXT_framebuffer_blit
#define GL_EXT_framebuffer_object
#define GL_EXT_histogram
#define GL_EXT_index_array_formats
#define GL_EXT_index_func
#define GL_EXT_index_material
#define GL_EXT_index_texture
#define GL_EXT_light_texture
#define GL_EXT_misc_attribute
#define GL_EXT_multi_draw_arrays
#define GL_EXT_multisample
#define GL_EXT_packed_pixels
#define GL_EXT_paletted_texture
#define GL_EXT_pixel_buffer_object
#define GL_EXT_pixel_transform
#define GL_EXT_pixel_transform_color_table
#define GL_EXT_point_parameters
#define GL_EXT_polygon_offset
#define GL_EXT_rescale_normal
#define GL_EXT_scene_marker
#define GL_EXT_secondary_color
#define GL_EXT_separate_specular_color
#define GL_EXT_shadow_funcs
#define GL_EXT_shared_texture_palette
#define GL_EXT_stencil_two_side
#define GL_EXT_stencil_wrap
#define GL_EXT_subtexture
#define GL_EXT_texture
#define GL_EXT_texture3D
#define GL_EXT_texture_compression_dxt1
#define GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_cube_map
#define GL_EXT_texture_edge_clamp
#define GL_EXT_texture_env
#define GL_EXT_texture_env_add
#define GL_EXT_texture_env_combine
#define GL_EXT_texture_env_dot3
#define GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_lod_bias
#define GL_EXT_texture_mirror_clamp
#define GL_EXT_texture_object
#define GL_EXT_texture_perturb_normal
#define GL_EXT_texture_rectangle
#define GL_EXT_vertex_array
#define GL_EXT_vertex_shader
#define GL_EXT_vertex_weighting
#define GL_GREMEDY_string_marker

#define GL_HP_convolution_border_modes
#define GL_HP_image_transform
#define GL_HP_occlusion_test
#define GL_HP_texture_lighting

#define GL_IBM_cull_vertex
#define GL_IBM_multimode_draw_arrays
#define GL_IBM_rasterpos_clip
#define GL_IBM_static_data
#define GL_IBM_texture_mirrored_repeat
#define GL_IBM_vertex_array_lists

#define GL_INGR_color_clamp
#define GL_INGR_interlace_read

#define GL_INTEL_parallel_arrays
#define GL_INTEL_texture_scissor

#define GL_KTX_buffer_region

#define GL_MESA_pack_invert
#define GL_MESA_resize_buffers
#define GL_MESA_window_pos
#define GL_MESA_ycbcr_texture

#define GL_NV_blend_square
#define GL_NV_copy_depth_to_color
#define GL_NV_depth_clamp
#define GL_NV_evaluators
#define GL_NV_fence
#define GL_NV_float_buffer
#define GL_NV_fog_distance
#define GL_NV_fragment_program
#define GL_NV_fragment_program2
#define GL_NV_fragment_program_option
#define GL_NV_half_float
#define GL_NV_light_max_exponent
#define GL_NV_multisample_filter_hint
#define GL_NV_occlusion_query
#define GL_NV_packed_depth_stencil
#define GL_NV_pixel_data_range
#define GL_NV_point_sprite
#define GL_NV_primitive_restart
#define GL_NV_register_combiners
#define GL_NV_register_combiners2
#define GL_NV_texgen_emboss
#define GL_NV_texgen_reflection
#define GL_NV_texture_compression_vtc
#define GL_NV_texture_env_combine4
#define GL_NV_texture_expand_normal
#define GL_NV_texture_rectangle
#define GL_NV_texture_shader
#define GL_NV_texture_shader2
#define GL_NV_texture_shader3
#define GL_NV_vertex_array_range
#define GL_NV_vertex_array_range2
#define GL_NV_vertex_program
#define GL_NV_vertex_program1_1
#define GL_NV_vertex_program2
#define GL_NV_vertex_program2_option
#define GL_NV_vertex_program3

#define GL_OML_interlace
#define GL_OML_resample
#define GL_OML_subsample

#define GL_PGI_misc_hints
#define GL_PGI_vertex_hints

#define GL_REND_screen_coordinates

#define GL_S3_s3tc

#define GL_SGIS_color_range
#define GL_SGIS_detail_texture
#define GL_SGIS_fog_function
#define GL_SGIS_generate_mipmap
#define GL_SGIS_multisample
#define GL_SGIS_pixel_texture
#define GL_SGIS_sharpen_texture
#define GL_SGIS_texture4D
#define GL_SGIS_texture_border_clamp
#define GL_SGIS_texture_edge_clamp
#define GL_SGIS_texture_filter4
#define GL_SGIS_texture_lod
#define GL_SGIS_texture_select

#define GL_SGIX_async
#define GL_SGIX_async_histogram
#define GL_SGIX_async_pixel
#define GL_SGIX_blend_alpha_minmax
#define GL_SGIX_clipmap
#define GL_SGIX_depth_texture
#define GL_SGIX_flush_raster
#define GL_SGIX_fog_offset
#define GL_SGIX_fog_texture
#define GL_SGIX_fragment_specular_lighting
#define GL_SGIX_framezoom
#define GL_SGIX_interlace
#define GL_SGIX_ir_instrument1
#define GL_SGIX_list_priority
#define GL_SGIX_pixel_texture
#define GL_SGIX_pixel_texture_bits
#define GL_SGIX_reference_plane
#define GL_SGIX_resample
#define GL_SGIX_shadow
#define GL_SGIX_shadow_ambient
#define GL_SGIX_sprite
#define GL_SGIX_tag_sample_buffer
#define GL_SGIX_texture_add_env
#define GL_SGIX_texture_coordinate_clamp
#define GL_SGIX_texture_lod_bias
#define GL_SGIX_texture_multi_buffer
#define GL_SGIX_texture_range
#define GL_SGIX_texture_scale_bias
#define GL_SGIX_vertex_preclip
#define GL_SGIX_vertex_preclip_hint
#define GL_SGIX_ycrcb

#define GL_SGI_color_matrix
#define GL_SGI_color_table
#define GL_SGI_texture_color_table

#define GL_SUNX_constant_data

#define GL_SUN_convolution_border_modes
#define GL_SUN_global_alpha
#define GL_SUN_mesh_array
#define GL_SUN_slice_accum
#define GL_SUN_triangle_list
#define GL_SUN_vertex

#define GL_WIN_phong_shading
#define GL_WIN_specular_fog
#define GL_WIN_swap_hint

#define GLU_EXT_nurbs_tessellator
#define GLU_EXT_object_space_tess
#define GLU_SGI_filter4_parameters

#define GLX_VERSION_1_1
#define GLX_VERSION_1_2
#define GLX_VERSION_1_3
#define GLX_VERSION_1_4
#define GLX_3DFX_multisample
#define GLX_ARB_fbconfig_float
#define GLX_ARB_get_proc_address
#define GLX_ARB_multisample
#define GLX_ATI_pixel_format_float
#define GLX_ATI_render_texture
#define GLX_EXT_import_context
#define GLX_EXT_scene_marker
#define GLX_EXT_visual_info
#define GLX_EXT_visual_rating
#define GLX_MESA_agp_offset
#define GLX_MESA_copy_sub_buffer
#define GLX_MESA_pixmap_colormap
#define GLX_MESA_release_buffers
#define GLX_MESA_set_3dfx_mode
#define GLX_NV_float_buffer
#define GLX_NV_vertex_array_range
#define GLX_OML_swap_method
#define GLX_OML_sync_control
#define GLX_SGIS_blended_overlay
#define GLX_SGIS_color_range
#define GLX_SGIS_multisample
#define GLX_SGIS_shared_multisample
#define GLX_SGIX_fbconfig
#define GLX_SGIX_pbuffer
#define GLX_SGIX_swap_barrier
#define GLX_SGIX_swap_group
#define GLX_SGIX_video_resize
#define GLX_SGIX_visual_select_group
#define GLX_SGI_cushion
#define GLX_SGI_make_current_read
#define GLX_SGI_swap_control
#define GLX_SGI_video_sync
#define GLX_SUN_get_transparent_index
#define GLX_SUN_video_resize

#define WGL_3DFX_multisample
#define WGL_ARB_buffer_region
#define WGL_ARB_extensions_string
#define WGL_ARB_make_current_read
#define WGL_ARB_multisample
#define WGL_ARB_pbuffer
#define WGL_ARB_pixel_format
#define WGL_ARB_pixel_format_float
#define WGL_ARB_render_texture
#define WGL_ATI_pixel_format_float
#define WGL_ATI_render_texture_rectangle
#define WGL_EXT_depth_float
#define WGL_EXT_display_color_table
#define WGL_EXT_extensions_string
#define WGL_EXT_make_current_read
#define WGL_EXT_multisample
#define WGL_EXT_pbuffer
#define WGL_EXT_pixel_format
#define WGL_EXT_swap_control
#define WGL_I3D_digital_video_control
#define WGL_I3D_gamma
#define WGL_I3D_genlock
#define WGL_I3D_image_buffer
#define WGL_I3D_swap_frame_lock
#define WGL_I3D_swap_frame_usage
#define WGL_NV_float_buffer
#define WGL_NV_render_depth_texture
#define WGL_NV_render_texture_rectangle
#define WGL_NV_vertex_array_range
#define WGL_OML_sync_control


