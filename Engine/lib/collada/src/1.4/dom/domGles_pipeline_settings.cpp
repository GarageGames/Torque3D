/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */

#include <dae.h>
#include <dae/daeDom.h>
#include <dom/domGles_pipeline_settings.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domGles_pipeline_settings::create(DAE& dae)
{
	domGles_pipeline_settingsRef ref = new domGles_pipeline_settings(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "gles_pipeline_settings" );
	meta->registerClass(domGles_pipeline_settings::create);

	meta->setIsTransparent( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "alpha_func" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemAlpha_func) );
	mea->setElementType( domGles_pipeline_settings::domAlpha_func::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "blend_func" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemBlend_func) );
	mea->setElementType( domGles_pipeline_settings::domBlend_func::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "clear_color" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemClear_color) );
	mea->setElementType( domGles_pipeline_settings::domClear_color::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "clear_stencil" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemClear_stencil) );
	mea->setElementType( domGles_pipeline_settings::domClear_stencil::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "clear_depth" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemClear_depth) );
	mea->setElementType( domGles_pipeline_settings::domClear_depth::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "clip_plane" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemClip_plane) );
	mea->setElementType( domGles_pipeline_settings::domClip_plane::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color_mask" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemColor_mask) );
	mea->setElementType( domGles_pipeline_settings::domColor_mask::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "cull_face" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemCull_face) );
	mea->setElementType( domGles_pipeline_settings::domCull_face::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "depth_func" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemDepth_func) );
	mea->setElementType( domGles_pipeline_settings::domDepth_func::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "depth_mask" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemDepth_mask) );
	mea->setElementType( domGles_pipeline_settings::domDepth_mask::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "depth_range" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemDepth_range) );
	mea->setElementType( domGles_pipeline_settings::domDepth_range::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fog_color" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemFog_color) );
	mea->setElementType( domGles_pipeline_settings::domFog_color::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fog_density" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemFog_density) );
	mea->setElementType( domGles_pipeline_settings::domFog_density::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fog_mode" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemFog_mode) );
	mea->setElementType( domGles_pipeline_settings::domFog_mode::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fog_start" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemFog_start) );
	mea->setElementType( domGles_pipeline_settings::domFog_start::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fog_end" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemFog_end) );
	mea->setElementType( domGles_pipeline_settings::domFog_end::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "front_face" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemFront_face) );
	mea->setElementType( domGles_pipeline_settings::domFront_face::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "texture_pipeline" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemTexture_pipeline) );
	mea->setElementType( domGles_pipeline_settings::domTexture_pipeline::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "logic_op" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLogic_op) );
	mea->setElementType( domGles_pipeline_settings::domLogic_op::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_ambient" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_ambient) );
	mea->setElementType( domGles_pipeline_settings::domLight_ambient::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_diffuse" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_diffuse) );
	mea->setElementType( domGles_pipeline_settings::domLight_diffuse::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_specular" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_specular) );
	mea->setElementType( domGles_pipeline_settings::domLight_specular::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_position" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_position) );
	mea->setElementType( domGles_pipeline_settings::domLight_position::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_constant_attenuation" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_constant_attenuation) );
	mea->setElementType( domGles_pipeline_settings::domLight_constant_attenuation::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_linear_attenutation" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_linear_attenutation) );
	mea->setElementType( domGles_pipeline_settings::domLight_linear_attenutation::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_quadratic_attenuation" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_quadratic_attenuation) );
	mea->setElementType( domGles_pipeline_settings::domLight_quadratic_attenuation::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_spot_cutoff" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_spot_cutoff) );
	mea->setElementType( domGles_pipeline_settings::domLight_spot_cutoff::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_spot_direction" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_spot_direction) );
	mea->setElementType( domGles_pipeline_settings::domLight_spot_direction::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_spot_exponent" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_spot_exponent) );
	mea->setElementType( domGles_pipeline_settings::domLight_spot_exponent::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_model_ambient" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_model_ambient) );
	mea->setElementType( domGles_pipeline_settings::domLight_model_ambient::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "line_width" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLine_width) );
	mea->setElementType( domGles_pipeline_settings::domLine_width::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "material_ambient" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemMaterial_ambient) );
	mea->setElementType( domGles_pipeline_settings::domMaterial_ambient::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "material_diffuse" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemMaterial_diffuse) );
	mea->setElementType( domGles_pipeline_settings::domMaterial_diffuse::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "material_emission" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemMaterial_emission) );
	mea->setElementType( domGles_pipeline_settings::domMaterial_emission::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "material_shininess" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemMaterial_shininess) );
	mea->setElementType( domGles_pipeline_settings::domMaterial_shininess::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "material_specular" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemMaterial_specular) );
	mea->setElementType( domGles_pipeline_settings::domMaterial_specular::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "model_view_matrix" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemModel_view_matrix) );
	mea->setElementType( domGles_pipeline_settings::domModel_view_matrix::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "point_distance_attenuation" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPoint_distance_attenuation) );
	mea->setElementType( domGles_pipeline_settings::domPoint_distance_attenuation::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "point_fade_threshold_size" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPoint_fade_threshold_size) );
	mea->setElementType( domGles_pipeline_settings::domPoint_fade_threshold_size::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "point_size" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPoint_size) );
	mea->setElementType( domGles_pipeline_settings::domPoint_size::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "point_size_min" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPoint_size_min) );
	mea->setElementType( domGles_pipeline_settings::domPoint_size_min::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "point_size_max" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPoint_size_max) );
	mea->setElementType( domGles_pipeline_settings::domPoint_size_max::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "polygon_offset" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPolygon_offset) );
	mea->setElementType( domGles_pipeline_settings::domPolygon_offset::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "projection_matrix" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemProjection_matrix) );
	mea->setElementType( domGles_pipeline_settings::domProjection_matrix::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "scissor" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemScissor) );
	mea->setElementType( domGles_pipeline_settings::domScissor::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "shade_model" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemShade_model) );
	mea->setElementType( domGles_pipeline_settings::domShade_model::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "stencil_func" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemStencil_func) );
	mea->setElementType( domGles_pipeline_settings::domStencil_func::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "stencil_mask" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemStencil_mask) );
	mea->setElementType( domGles_pipeline_settings::domStencil_mask::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "stencil_op" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemStencil_op) );
	mea->setElementType( domGles_pipeline_settings::domStencil_op::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "alpha_test_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemAlpha_test_enable) );
	mea->setElementType( domGles_pipeline_settings::domAlpha_test_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "blend_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemBlend_enable) );
	mea->setElementType( domGles_pipeline_settings::domBlend_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "clip_plane_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemClip_plane_enable) );
	mea->setElementType( domGles_pipeline_settings::domClip_plane_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color_logic_op_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemColor_logic_op_enable) );
	mea->setElementType( domGles_pipeline_settings::domColor_logic_op_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color_material_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemColor_material_enable) );
	mea->setElementType( domGles_pipeline_settings::domColor_material_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "cull_face_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemCull_face_enable) );
	mea->setElementType( domGles_pipeline_settings::domCull_face_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "depth_test_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemDepth_test_enable) );
	mea->setElementType( domGles_pipeline_settings::domDepth_test_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "dither_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemDither_enable) );
	mea->setElementType( domGles_pipeline_settings::domDither_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fog_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemFog_enable) );
	mea->setElementType( domGles_pipeline_settings::domFog_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "texture_pipeline_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemTexture_pipeline_enable) );
	mea->setElementType( domGles_pipeline_settings::domTexture_pipeline_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_enable) );
	mea->setElementType( domGles_pipeline_settings::domLight_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "lighting_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLighting_enable) );
	mea->setElementType( domGles_pipeline_settings::domLighting_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "light_model_two_side_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLight_model_two_side_enable) );
	mea->setElementType( domGles_pipeline_settings::domLight_model_two_side_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "line_smooth_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemLine_smooth_enable) );
	mea->setElementType( domGles_pipeline_settings::domLine_smooth_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "multisample_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemMultisample_enable) );
	mea->setElementType( domGles_pipeline_settings::domMultisample_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "normalize_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemNormalize_enable) );
	mea->setElementType( domGles_pipeline_settings::domNormalize_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "point_smooth_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPoint_smooth_enable) );
	mea->setElementType( domGles_pipeline_settings::domPoint_smooth_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "polygon_offset_fill_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemPolygon_offset_fill_enable) );
	mea->setElementType( domGles_pipeline_settings::domPolygon_offset_fill_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "rescale_normal_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemRescale_normal_enable) );
	mea->setElementType( domGles_pipeline_settings::domRescale_normal_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sample_alpha_to_coverage_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemSample_alpha_to_coverage_enable) );
	mea->setElementType( domGles_pipeline_settings::domSample_alpha_to_coverage_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sample_alpha_to_one_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemSample_alpha_to_one_enable) );
	mea->setElementType( domGles_pipeline_settings::domSample_alpha_to_one_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sample_coverage_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemSample_coverage_enable) );
	mea->setElementType( domGles_pipeline_settings::domSample_coverage_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "scissor_test_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemScissor_test_enable) );
	mea->setElementType( domGles_pipeline_settings::domScissor_test_enable::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "stencil_test_enable" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings,elemStencil_test_enable) );
	mea->setElementType( domGles_pipeline_settings::domStencil_test_enable::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domGles_pipeline_settings,_contents));
	meta->addContentsOrder(daeOffsetOf(domGles_pipeline_settings,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domGles_pipeline_settings,_CMData), 1);
	meta->setElementSize(sizeof(domGles_pipeline_settings));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domAlpha_func::create(DAE& dae)
{
	domGles_pipeline_settings::domAlpha_funcRef ref = new domGles_pipeline_settings::domAlpha_func(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domAlpha_func::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "alpha_func" );
	meta->registerClass(domGles_pipeline_settings::domAlpha_func::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "func" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domAlpha_func,elemFunc) );
	mea->setElementType( domGles_pipeline_settings::domAlpha_func::domFunc::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "value" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domAlpha_func,elemValue) );
	mea->setElementType( domGles_pipeline_settings::domAlpha_func::domValue::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domGles_pipeline_settings::domAlpha_func));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domAlpha_func::domFunc::create(DAE& dae)
{
	domGles_pipeline_settings::domAlpha_func::domFuncRef ref = new domGles_pipeline_settings::domAlpha_func::domFunc(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domAlpha_func::domFunc::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "func" );
	meta->registerClass(domGles_pipeline_settings::domAlpha_func::domFunc::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_func_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domAlpha_func::domFunc , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "ALWAYS");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domAlpha_func::domFunc , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domAlpha_func::domFunc));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domAlpha_func::domValue::create(DAE& dae)
{
	domGles_pipeline_settings::domAlpha_func::domValueRef ref = new domGles_pipeline_settings::domAlpha_func::domValue(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domAlpha_func::domValue::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "value" );
	meta->registerClass(domGles_pipeline_settings::domAlpha_func::domValue::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_alpha_value_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domAlpha_func::domValue , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0.0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domAlpha_func::domValue , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domAlpha_func::domValue));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domBlend_func::create(DAE& dae)
{
	domGles_pipeline_settings::domBlend_funcRef ref = new domGles_pipeline_settings::domBlend_func(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domBlend_func::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "blend_func" );
	meta->registerClass(domGles_pipeline_settings::domBlend_func::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "src" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domBlend_func,elemSrc) );
	mea->setElementType( domGles_pipeline_settings::domBlend_func::domSrc::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "dest" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domBlend_func,elemDest) );
	mea->setElementType( domGles_pipeline_settings::domBlend_func::domDest::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domGles_pipeline_settings::domBlend_func));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domBlend_func::domSrc::create(DAE& dae)
{
	domGles_pipeline_settings::domBlend_func::domSrcRef ref = new domGles_pipeline_settings::domBlend_func::domSrc(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domBlend_func::domSrc::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "src" );
	meta->registerClass(domGles_pipeline_settings::domBlend_func::domSrc::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_blend_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domBlend_func::domSrc , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "ONE");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domBlend_func::domSrc , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domBlend_func::domSrc));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domBlend_func::domDest::create(DAE& dae)
{
	domGles_pipeline_settings::domBlend_func::domDestRef ref = new domGles_pipeline_settings::domBlend_func::domDest(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domBlend_func::domDest::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "dest" );
	meta->registerClass(domGles_pipeline_settings::domBlend_func::domDest::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_blend_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domBlend_func::domDest , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "ZERO");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domBlend_func::domDest , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domBlend_func::domDest));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domClear_color::create(DAE& dae)
{
	domGles_pipeline_settings::domClear_colorRef ref = new domGles_pipeline_settings::domClear_color(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domClear_color::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "clear_color" );
	meta->registerClass(domGles_pipeline_settings::domClear_color::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClear_color , attrValue ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClear_color , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domClear_color));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domClear_stencil::create(DAE& dae)
{
	domGles_pipeline_settings::domClear_stencilRef ref = new domGles_pipeline_settings::domClear_stencil(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domClear_stencil::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "clear_stencil" );
	meta->registerClass(domGles_pipeline_settings::domClear_stencil::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Int"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClear_stencil , attrValue ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClear_stencil , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domClear_stencil));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domClear_depth::create(DAE& dae)
{
	domGles_pipeline_settings::domClear_depthRef ref = new domGles_pipeline_settings::domClear_depth(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domClear_depth::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "clear_depth" );
	meta->registerClass(domGles_pipeline_settings::domClear_depth::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClear_depth , attrValue ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClear_depth , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domClear_depth));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domClip_plane::create(DAE& dae)
{
	domGles_pipeline_settings::domClip_planeRef ref = new domGles_pipeline_settings::domClip_plane(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domClip_plane::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "clip_plane" );
	meta->registerClass(domGles_pipeline_settings::domClip_plane::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClip_plane , attrValue ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClip_plane , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_CLIP_PLANES_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClip_plane , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domClip_plane));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domColor_mask::create(DAE& dae)
{
	domGles_pipeline_settings::domColor_maskRef ref = new domGles_pipeline_settings::domColor_mask(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domColor_mask::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "color_mask" );
	meta->registerClass(domGles_pipeline_settings::domColor_mask::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domColor_mask , attrValue ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domColor_mask , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domColor_mask));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domCull_face::create(DAE& dae)
{
	domGles_pipeline_settings::domCull_faceRef ref = new domGles_pipeline_settings::domCull_face(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domCull_face::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "cull_face" );
	meta->registerClass(domGles_pipeline_settings::domCull_face::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_face_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domCull_face , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "BACK");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domCull_face , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domCull_face));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domDepth_func::create(DAE& dae)
{
	domGles_pipeline_settings::domDepth_funcRef ref = new domGles_pipeline_settings::domDepth_func(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domDepth_func::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "depth_func" );
	meta->registerClass(domGles_pipeline_settings::domDepth_func::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_func_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_func , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "ALWAYS");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_func , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domDepth_func));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domDepth_mask::create(DAE& dae)
{
	domGles_pipeline_settings::domDepth_maskRef ref = new domGles_pipeline_settings::domDepth_mask(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domDepth_mask::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "depth_mask" );
	meta->registerClass(domGles_pipeline_settings::domDepth_mask::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_mask , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_mask , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domDepth_mask));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domDepth_range::create(DAE& dae)
{
	domGles_pipeline_settings::domDepth_rangeRef ref = new domGles_pipeline_settings::domDepth_range(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domDepth_range::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "depth_range" );
	meta->registerClass(domGles_pipeline_settings::domDepth_range::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float2"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_range , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_range , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domDepth_range));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domFog_color::create(DAE& dae)
{
	domGles_pipeline_settings::domFog_colorRef ref = new domGles_pipeline_settings::domFog_color(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domFog_color::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fog_color" );
	meta->registerClass(domGles_pipeline_settings::domFog_color::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_color , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 0 0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_color , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domFog_color));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domFog_density::create(DAE& dae)
{
	domGles_pipeline_settings::domFog_densityRef ref = new domGles_pipeline_settings::domFog_density(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domFog_density::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fog_density" );
	meta->registerClass(domGles_pipeline_settings::domFog_density::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_density , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_density , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domFog_density));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domFog_mode::create(DAE& dae)
{
	domGles_pipeline_settings::domFog_modeRef ref = new domGles_pipeline_settings::domFog_mode(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domFog_mode::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fog_mode" );
	meta->registerClass(domGles_pipeline_settings::domFog_mode::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_fog_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_mode , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "EXP");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_mode , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domFog_mode));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domFog_start::create(DAE& dae)
{
	domGles_pipeline_settings::domFog_startRef ref = new domGles_pipeline_settings::domFog_start(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domFog_start::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fog_start" );
	meta->registerClass(domGles_pipeline_settings::domFog_start::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_start , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_start , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domFog_start));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domFog_end::create(DAE& dae)
{
	domGles_pipeline_settings::domFog_endRef ref = new domGles_pipeline_settings::domFog_end(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domFog_end::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fog_end" );
	meta->registerClass(domGles_pipeline_settings::domFog_end::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_end , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_end , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domFog_end));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domFront_face::create(DAE& dae)
{
	domGles_pipeline_settings::domFront_faceRef ref = new domGles_pipeline_settings::domFront_face(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domFront_face::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "front_face" );
	meta->registerClass(domGles_pipeline_settings::domFront_face::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_front_face_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFront_face , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "CCW");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFront_face , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domFront_face));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domTexture_pipeline::create(DAE& dae)
{
	domGles_pipeline_settings::domTexture_pipelineRef ref = new domGles_pipeline_settings::domTexture_pipeline(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domTexture_pipeline::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "texture_pipeline" );
	meta->registerClass(domGles_pipeline_settings::domTexture_pipeline::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "value" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domTexture_pipeline,elemValue) );
	mea->setElementType( domGles_texture_pipeline::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domTexture_pipeline , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domTexture_pipeline));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLogic_op::create(DAE& dae)
{
	domGles_pipeline_settings::domLogic_opRef ref = new domGles_pipeline_settings::domLogic_op(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLogic_op::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "logic_op" );
	meta->registerClass(domGles_pipeline_settings::domLogic_op::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_logic_op_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLogic_op , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "COPY");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLogic_op , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLogic_op));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_ambient::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_ambientRef ref = new domGles_pipeline_settings::domLight_ambient(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_ambient::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_ambient" );
	meta->registerClass(domGles_pipeline_settings::domLight_ambient::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_ambient , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 0 1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_ambient , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_ambient , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_ambient));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_diffuse::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_diffuseRef ref = new domGles_pipeline_settings::domLight_diffuse(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_diffuse::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_diffuse" );
	meta->registerClass(domGles_pipeline_settings::domLight_diffuse::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_diffuse , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 0 0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_diffuse , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_diffuse , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_diffuse));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_specular::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_specularRef ref = new domGles_pipeline_settings::domLight_specular(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_specular::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_specular" );
	meta->registerClass(domGles_pipeline_settings::domLight_specular::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_specular , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 0 0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_specular , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_specular , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_specular));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_position::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_positionRef ref = new domGles_pipeline_settings::domLight_position(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_position::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_position" );
	meta->registerClass(domGles_pipeline_settings::domLight_position::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_position , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 1 0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_position , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_position , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_position));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_constant_attenuation::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_constant_attenuationRef ref = new domGles_pipeline_settings::domLight_constant_attenuation(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_constant_attenuation::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_constant_attenuation" );
	meta->registerClass(domGles_pipeline_settings::domLight_constant_attenuation::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_constant_attenuation , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_constant_attenuation , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_constant_attenuation , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_constant_attenuation));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_linear_attenutation::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_linear_attenutationRef ref = new domGles_pipeline_settings::domLight_linear_attenutation(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_linear_attenutation::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_linear_attenutation" );
	meta->registerClass(domGles_pipeline_settings::domLight_linear_attenutation::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_linear_attenutation , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_linear_attenutation , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_linear_attenutation , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_linear_attenutation));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_quadratic_attenuation::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_quadratic_attenuationRef ref = new domGles_pipeline_settings::domLight_quadratic_attenuation(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_quadratic_attenuation::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_quadratic_attenuation" );
	meta->registerClass(domGles_pipeline_settings::domLight_quadratic_attenuation::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_quadratic_attenuation , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_quadratic_attenuation , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_quadratic_attenuation , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_quadratic_attenuation));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_spot_cutoff::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_spot_cutoffRef ref = new domGles_pipeline_settings::domLight_spot_cutoff(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_spot_cutoff::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_spot_cutoff" );
	meta->registerClass(domGles_pipeline_settings::domLight_spot_cutoff::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_cutoff , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "180");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_cutoff , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_cutoff , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_spot_cutoff));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_spot_direction::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_spot_directionRef ref = new domGles_pipeline_settings::domLight_spot_direction(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_spot_direction::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_spot_direction" );
	meta->registerClass(domGles_pipeline_settings::domLight_spot_direction::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float3"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_direction , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 -1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_direction , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_direction , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_spot_direction));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_spot_exponent::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_spot_exponentRef ref = new domGles_pipeline_settings::domLight_spot_exponent(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_spot_exponent::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_spot_exponent" );
	meta->registerClass(domGles_pipeline_settings::domLight_spot_exponent::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_exponent , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_exponent , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_spot_exponent , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_spot_exponent));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_model_ambient::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_model_ambientRef ref = new domGles_pipeline_settings::domLight_model_ambient(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_model_ambient::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_model_ambient" );
	meta->registerClass(domGles_pipeline_settings::domLight_model_ambient::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_model_ambient , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0.2 0.2 0.2 1.0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_model_ambient , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_model_ambient));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLine_width::create(DAE& dae)
{
	domGles_pipeline_settings::domLine_widthRef ref = new domGles_pipeline_settings::domLine_width(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLine_width::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "line_width" );
	meta->registerClass(domGles_pipeline_settings::domLine_width::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLine_width , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLine_width , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLine_width));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domMaterial_ambient::create(DAE& dae)
{
	domGles_pipeline_settings::domMaterial_ambientRef ref = new domGles_pipeline_settings::domMaterial_ambient(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domMaterial_ambient::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "material_ambient" );
	meta->registerClass(domGles_pipeline_settings::domMaterial_ambient::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_ambient , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0.2 0.2 0.2 1.0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_ambient , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domMaterial_ambient));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domMaterial_diffuse::create(DAE& dae)
{
	domGles_pipeline_settings::domMaterial_diffuseRef ref = new domGles_pipeline_settings::domMaterial_diffuse(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domMaterial_diffuse::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "material_diffuse" );
	meta->registerClass(domGles_pipeline_settings::domMaterial_diffuse::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_diffuse , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0.8 0.8 0.8 1.0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_diffuse , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domMaterial_diffuse));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domMaterial_emission::create(DAE& dae)
{
	domGles_pipeline_settings::domMaterial_emissionRef ref = new domGles_pipeline_settings::domMaterial_emission(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domMaterial_emission::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "material_emission" );
	meta->registerClass(domGles_pipeline_settings::domMaterial_emission::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_emission , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 0 1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_emission , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domMaterial_emission));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domMaterial_shininess::create(DAE& dae)
{
	domGles_pipeline_settings::domMaterial_shininessRef ref = new domGles_pipeline_settings::domMaterial_shininess(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domMaterial_shininess::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "material_shininess" );
	meta->registerClass(domGles_pipeline_settings::domMaterial_shininess::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_shininess , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_shininess , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domMaterial_shininess));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domMaterial_specular::create(DAE& dae)
{
	domGles_pipeline_settings::domMaterial_specularRef ref = new domGles_pipeline_settings::domMaterial_specular(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domMaterial_specular::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "material_specular" );
	meta->registerClass(domGles_pipeline_settings::domMaterial_specular::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_specular , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0 0 1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMaterial_specular , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domMaterial_specular));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domModel_view_matrix::create(DAE& dae)
{
	domGles_pipeline_settings::domModel_view_matrixRef ref = new domGles_pipeline_settings::domModel_view_matrix(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domModel_view_matrix::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "model_view_matrix" );
	meta->registerClass(domGles_pipeline_settings::domModel_view_matrix::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4x4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domModel_view_matrix , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domModel_view_matrix , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domModel_view_matrix));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPoint_distance_attenuation::create(DAE& dae)
{
	domGles_pipeline_settings::domPoint_distance_attenuationRef ref = new domGles_pipeline_settings::domPoint_distance_attenuation(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPoint_distance_attenuation::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "point_distance_attenuation" );
	meta->registerClass(domGles_pipeline_settings::domPoint_distance_attenuation::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float3"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_distance_attenuation , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1 0 0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_distance_attenuation , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPoint_distance_attenuation));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPoint_fade_threshold_size::create(DAE& dae)
{
	domGles_pipeline_settings::domPoint_fade_threshold_sizeRef ref = new domGles_pipeline_settings::domPoint_fade_threshold_size(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPoint_fade_threshold_size::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "point_fade_threshold_size" );
	meta->registerClass(domGles_pipeline_settings::domPoint_fade_threshold_size::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_fade_threshold_size , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_fade_threshold_size , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPoint_fade_threshold_size));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPoint_size::create(DAE& dae)
{
	domGles_pipeline_settings::domPoint_sizeRef ref = new domGles_pipeline_settings::domPoint_size(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPoint_size::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "point_size" );
	meta->registerClass(domGles_pipeline_settings::domPoint_size::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_size , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_size , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPoint_size));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPoint_size_min::create(DAE& dae)
{
	domGles_pipeline_settings::domPoint_size_minRef ref = new domGles_pipeline_settings::domPoint_size_min(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPoint_size_min::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "point_size_min" );
	meta->registerClass(domGles_pipeline_settings::domPoint_size_min::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_size_min , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_size_min , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPoint_size_min));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPoint_size_max::create(DAE& dae)
{
	domGles_pipeline_settings::domPoint_size_maxRef ref = new domGles_pipeline_settings::domPoint_size_max(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPoint_size_max::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "point_size_max" );
	meta->registerClass(domGles_pipeline_settings::domPoint_size_max::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_size_max , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_size_max , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPoint_size_max));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPolygon_offset::create(DAE& dae)
{
	domGles_pipeline_settings::domPolygon_offsetRef ref = new domGles_pipeline_settings::domPolygon_offset(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPolygon_offset::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "polygon_offset" );
	meta->registerClass(domGles_pipeline_settings::domPolygon_offset::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float2"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPolygon_offset , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0 0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPolygon_offset , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPolygon_offset));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domProjection_matrix::create(DAE& dae)
{
	domGles_pipeline_settings::domProjection_matrixRef ref = new domGles_pipeline_settings::domProjection_matrix(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domProjection_matrix::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "projection_matrix" );
	meta->registerClass(domGles_pipeline_settings::domProjection_matrix::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Float4x4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domProjection_matrix , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domProjection_matrix , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domProjection_matrix));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domScissor::create(DAE& dae)
{
	domGles_pipeline_settings::domScissorRef ref = new domGles_pipeline_settings::domScissor(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domScissor::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "scissor" );
	meta->registerClass(domGles_pipeline_settings::domScissor::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Int4"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domScissor , attrValue ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domScissor , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domScissor));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domShade_model::create(DAE& dae)
{
	domGles_pipeline_settings::domShade_modelRef ref = new domGles_pipeline_settings::domShade_model(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domShade_model::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "shade_model" );
	meta->registerClass(domGles_pipeline_settings::domShade_model::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_shade_model_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domShade_model , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "SMOOTH");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domShade_model , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domShade_model));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_func::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_funcRef ref = new domGles_pipeline_settings::domStencil_func(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_func::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "stencil_func" );
	meta->registerClass(domGles_pipeline_settings::domStencil_func::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "func" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domStencil_func,elemFunc) );
	mea->setElementType( domGles_pipeline_settings::domStencil_func::domFunc::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "ref" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domStencil_func,elemRef) );
	mea->setElementType( domGles_pipeline_settings::domStencil_func::domRef::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 1, 1 );
	mea->setName( "mask" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domStencil_func,elemMask) );
	mea->setElementType( domGles_pipeline_settings::domStencil_func::domMask::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_func));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_func::domFunc::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_func::domFuncRef ref = new domGles_pipeline_settings::domStencil_func::domFunc(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_func::domFunc::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "func" );
	meta->registerClass(domGles_pipeline_settings::domStencil_func::domFunc::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gl_func_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_func::domFunc , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "ALWAYS");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_func::domFunc , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_func::domFunc));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_func::domRef::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_func::domRefRef ref = new domGles_pipeline_settings::domStencil_func::domRef(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_func::domRef::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "ref" );
	meta->registerClass(domGles_pipeline_settings::domStencil_func::domRef::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("xsUnsignedByte"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_func::domRef , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "0");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_func::domRef , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_func::domRef));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_func::domMask::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_func::domMaskRef ref = new domGles_pipeline_settings::domStencil_func::domMask(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_func::domMask::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "mask" );
	meta->registerClass(domGles_pipeline_settings::domStencil_func::domMask::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("xsUnsignedByte"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_func::domMask , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "255");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_func::domMask , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_func::domMask));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_mask::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_maskRef ref = new domGles_pipeline_settings::domStencil_mask(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_mask::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "stencil_mask" );
	meta->registerClass(domGles_pipeline_settings::domStencil_mask::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Int"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_mask , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "4294967295");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_mask , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_mask));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_op::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_opRef ref = new domGles_pipeline_settings::domStencil_op(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_op::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "stencil_op" );
	meta->registerClass(domGles_pipeline_settings::domStencil_op::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fail" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domStencil_op,elemFail) );
	mea->setElementType( domGles_pipeline_settings::domStencil_op::domFail::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "zfail" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domStencil_op,elemZfail) );
	mea->setElementType( domGles_pipeline_settings::domStencil_op::domZfail::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 1, 1 );
	mea->setName( "zpass" );
	mea->setOffset( daeOffsetOf(domGles_pipeline_settings::domStencil_op,elemZpass) );
	mea->setElementType( domGles_pipeline_settings::domStencil_op::domZpass::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_op));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_op::domFail::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_op::domFailRef ref = new domGles_pipeline_settings::domStencil_op::domFail(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_op::domFail::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fail" );
	meta->registerClass(domGles_pipeline_settings::domStencil_op::domFail::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gles_stencil_op_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_op::domFail , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "KEEP");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_op::domFail , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_op::domFail));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_op::domZfail::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_op::domZfailRef ref = new domGles_pipeline_settings::domStencil_op::domZfail(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_op::domZfail::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "zfail" );
	meta->registerClass(domGles_pipeline_settings::domStencil_op::domZfail::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gles_stencil_op_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_op::domZfail , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "KEEP");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_op::domZfail , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_op::domZfail));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_op::domZpass::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_op::domZpassRef ref = new domGles_pipeline_settings::domStencil_op::domZpass(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_op::domZpass::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "zpass" );
	meta->registerClass(domGles_pipeline_settings::domStencil_op::domZpass::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Gles_stencil_op_type"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_op::domZpass , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "KEEP");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_op::domZpass , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_op::domZpass));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domAlpha_test_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domAlpha_test_enableRef ref = new domGles_pipeline_settings::domAlpha_test_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domAlpha_test_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "alpha_test_enable" );
	meta->registerClass(domGles_pipeline_settings::domAlpha_test_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domAlpha_test_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domAlpha_test_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domAlpha_test_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domBlend_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domBlend_enableRef ref = new domGles_pipeline_settings::domBlend_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domBlend_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "blend_enable" );
	meta->registerClass(domGles_pipeline_settings::domBlend_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domBlend_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domBlend_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domBlend_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domClip_plane_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domClip_plane_enableRef ref = new domGles_pipeline_settings::domClip_plane_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domClip_plane_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "clip_plane_enable" );
	meta->registerClass(domGles_pipeline_settings::domClip_plane_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClip_plane_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClip_plane_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_CLIP_PLANES_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domClip_plane_enable , attrIndex ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domClip_plane_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domColor_logic_op_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domColor_logic_op_enableRef ref = new domGles_pipeline_settings::domColor_logic_op_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domColor_logic_op_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "color_logic_op_enable" );
	meta->registerClass(domGles_pipeline_settings::domColor_logic_op_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domColor_logic_op_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domColor_logic_op_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domColor_logic_op_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domColor_material_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domColor_material_enableRef ref = new domGles_pipeline_settings::domColor_material_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domColor_material_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "color_material_enable" );
	meta->registerClass(domGles_pipeline_settings::domColor_material_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domColor_material_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "true");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domColor_material_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domColor_material_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domCull_face_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domCull_face_enableRef ref = new domGles_pipeline_settings::domCull_face_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domCull_face_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "cull_face_enable" );
	meta->registerClass(domGles_pipeline_settings::domCull_face_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domCull_face_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domCull_face_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domCull_face_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domDepth_test_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domDepth_test_enableRef ref = new domGles_pipeline_settings::domDepth_test_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domDepth_test_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "depth_test_enable" );
	meta->registerClass(domGles_pipeline_settings::domDepth_test_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_test_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDepth_test_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domDepth_test_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domDither_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domDither_enableRef ref = new domGles_pipeline_settings::domDither_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domDither_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "dither_enable" );
	meta->registerClass(domGles_pipeline_settings::domDither_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDither_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domDither_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domDither_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domFog_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domFog_enableRef ref = new domGles_pipeline_settings::domFog_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domFog_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fog_enable" );
	meta->registerClass(domGles_pipeline_settings::domFog_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domFog_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domFog_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domTexture_pipeline_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domTexture_pipeline_enableRef ref = new domGles_pipeline_settings::domTexture_pipeline_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domTexture_pipeline_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "texture_pipeline_enable" );
	meta->registerClass(domGles_pipeline_settings::domTexture_pipeline_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domTexture_pipeline_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domTexture_pipeline_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domTexture_pipeline_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_enableRef ref = new domGles_pipeline_settings::domLight_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_enable" );
	meta->registerClass(domGles_pipeline_settings::domLight_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: index
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "index" );
		ma->setType( dae.getAtomicTypes().get("GLES_MAX_LIGHTS_index"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_enable , attrIndex ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLighting_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domLighting_enableRef ref = new domGles_pipeline_settings::domLighting_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLighting_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "lighting_enable" );
	meta->registerClass(domGles_pipeline_settings::domLighting_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLighting_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLighting_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLighting_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLight_model_two_side_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domLight_model_two_side_enableRef ref = new domGles_pipeline_settings::domLight_model_two_side_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLight_model_two_side_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light_model_two_side_enable" );
	meta->registerClass(domGles_pipeline_settings::domLight_model_two_side_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_model_two_side_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLight_model_two_side_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLight_model_two_side_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domLine_smooth_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domLine_smooth_enableRef ref = new domGles_pipeline_settings::domLine_smooth_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domLine_smooth_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "line_smooth_enable" );
	meta->registerClass(domGles_pipeline_settings::domLine_smooth_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLine_smooth_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domLine_smooth_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domLine_smooth_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domMultisample_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domMultisample_enableRef ref = new domGles_pipeline_settings::domMultisample_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domMultisample_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "multisample_enable" );
	meta->registerClass(domGles_pipeline_settings::domMultisample_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMultisample_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domMultisample_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domMultisample_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domNormalize_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domNormalize_enableRef ref = new domGles_pipeline_settings::domNormalize_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domNormalize_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "normalize_enable" );
	meta->registerClass(domGles_pipeline_settings::domNormalize_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domNormalize_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domNormalize_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domNormalize_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPoint_smooth_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domPoint_smooth_enableRef ref = new domGles_pipeline_settings::domPoint_smooth_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPoint_smooth_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "point_smooth_enable" );
	meta->registerClass(domGles_pipeline_settings::domPoint_smooth_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_smooth_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPoint_smooth_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPoint_smooth_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domPolygon_offset_fill_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domPolygon_offset_fill_enableRef ref = new domGles_pipeline_settings::domPolygon_offset_fill_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domPolygon_offset_fill_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "polygon_offset_fill_enable" );
	meta->registerClass(domGles_pipeline_settings::domPolygon_offset_fill_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPolygon_offset_fill_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domPolygon_offset_fill_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domPolygon_offset_fill_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domRescale_normal_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domRescale_normal_enableRef ref = new domGles_pipeline_settings::domRescale_normal_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domRescale_normal_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "rescale_normal_enable" );
	meta->registerClass(domGles_pipeline_settings::domRescale_normal_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domRescale_normal_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domRescale_normal_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domRescale_normal_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domSample_alpha_to_coverage_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domSample_alpha_to_coverage_enableRef ref = new domGles_pipeline_settings::domSample_alpha_to_coverage_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domSample_alpha_to_coverage_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "sample_alpha_to_coverage_enable" );
	meta->registerClass(domGles_pipeline_settings::domSample_alpha_to_coverage_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domSample_alpha_to_coverage_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domSample_alpha_to_coverage_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domSample_alpha_to_coverage_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domSample_alpha_to_one_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domSample_alpha_to_one_enableRef ref = new domGles_pipeline_settings::domSample_alpha_to_one_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domSample_alpha_to_one_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "sample_alpha_to_one_enable" );
	meta->registerClass(domGles_pipeline_settings::domSample_alpha_to_one_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domSample_alpha_to_one_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domSample_alpha_to_one_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domSample_alpha_to_one_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domSample_coverage_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domSample_coverage_enableRef ref = new domGles_pipeline_settings::domSample_coverage_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domSample_coverage_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "sample_coverage_enable" );
	meta->registerClass(domGles_pipeline_settings::domSample_coverage_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domSample_coverage_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domSample_coverage_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domSample_coverage_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domScissor_test_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domScissor_test_enableRef ref = new domGles_pipeline_settings::domScissor_test_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domScissor_test_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "scissor_test_enable" );
	meta->registerClass(domGles_pipeline_settings::domScissor_test_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domScissor_test_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domScissor_test_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domScissor_test_enable));
	meta->validate();

	return meta;
}

daeElementRef
domGles_pipeline_settings::domStencil_test_enable::create(DAE& dae)
{
	domGles_pipeline_settings::domStencil_test_enableRef ref = new domGles_pipeline_settings::domStencil_test_enable(dae);
	return ref;
}


daeMetaElement *
domGles_pipeline_settings::domStencil_test_enable::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "stencil_test_enable" );
	meta->registerClass(domGles_pipeline_settings::domStencil_test_enable::create);

	meta->setIsInnerClass( true );

	//	Add attribute: value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_test_enable , attrValue ));
		ma->setContainer( meta );
		ma->setDefaultString( "false");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: param
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "param" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_pipeline_settings::domStencil_test_enable , attrParam ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_pipeline_settings::domStencil_test_enable));
	meta->validate();

	return meta;
}

