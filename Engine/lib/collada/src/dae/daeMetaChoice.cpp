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

#include <dae/daeMetaChoice.h>
#include <dae/daeMetaElement.h>

daeMetaChoice::daeMetaChoice( daeMetaElement *container, daeMetaCMPolicy *parent, daeUInt choiceNum, daeUInt ordinal,
												 daeInt minO, daeInt maxO) : daeMetaCMPolicy( container, parent, ordinal, minO, maxO ), _choiceNum(choiceNum)
{}

daeMetaChoice::~daeMetaChoice()
{}

daeElement *daeMetaChoice::placeElement( daeElement *parent, daeElement *child, daeUInt &ordinal, daeInt offset, daeElement* before, daeElement *after ) {
	(void)offset;
	if ( _maxOccurs == -1 ) {
		//Needed to prevent infinate loops. If unbounded check to see if you have the child before just trying to place
		if ( findChild( child->getElementName() ) == NULL ) {
			return NULL;
		}
	}

	daeElement *retVal = NULL;
	size_t cnt = _children.getCount();

	daeTArray< daeCharArray *> *CMData = (daeTArray< daeCharArray *>*)_container->getMetaCMData()->getWritableMemory(parent);
	daeCharArray *myData = CMData->get( _choiceNum );

	for ( daeInt i = 0; ( i < _maxOccurs || _maxOccurs == -1 ); i++ ) 
	{
		if ( (daeInt)myData->getCount() > i && myData->get(i) != -1 ) //choice has already been made
		{
			if ( _children[ myData->get(i) ]->placeElement( parent, child, ordinal, i, before, after ) != NULL ) 
			{
				retVal = child;
				ordinal = ordinal  + _ordinalOffset;
				break;
			}
			//else //try to see if everything can be in a different choice
			//{
			//	daeElementRefArray childsInChoice;
			//	_children[ myData->get(i) ]->getChildren( parent, childsInChoice );
			//	for ( size_t x = myData->get(i) +1; x < cnt; x++ )
			//	{
			//		daeElementRefArray childsInNext;
			//		_children[ x ]->getChildren( parent, childsInNext ); //If you get children in another choice then
			//		//both choices can have the same type of children.
			//		if ( childsInNext.getCount() == childsInChoice.getCount() ) 
			//		{
			//			//if there are the same ammount of children then all present children can belong to both
			//			//choices. Try to place the new child in this next choice.
			//			if ( _children[x]->placeElement( parent, child, ordinal, i, before, after ) != NULL ) 
			//			{
			//				retVal = child;
			//				ordinal = ordinal  + _ordinalOffset;

			//				myData->set( i, (daeChar)x ); //change the choice to this new one
			//				break;
			//			}
			//		}
			//	}
			//	if ( retVal != NULL ) break;
			//}
		}
		else //no choice has been made yet
		{
			for ( size_t x = 0; x < cnt; x++ ) 
			{
				if ( _children[x]->placeElement( parent, child, ordinal, i, before, after ) != NULL ) 
				{
					retVal = child;
					ordinal = ordinal  + _ordinalOffset;

					myData->append( (daeChar)x ); //you always place in the next available choice up to maxOccurs
					break;
				}
			}
			if ( retVal != NULL ) break;
		}
	}
	if ( retVal == NULL )
	{
		if ( findChild( child->getElementName() ) == NULL ) {
			return NULL;
		}
		for ( daeInt i = 0; ( i < _maxOccurs || _maxOccurs == -1 ); i++ ) 
		{
			daeElementRefArray childsInChoice;
			_children[ myData->get(i) ]->getChildren( parent, childsInChoice );
			for ( size_t x = myData->get(i) +1; x < cnt; x++ )
			{
				daeElementRefArray childsInNext;
				_children[ x ]->getChildren( parent, childsInNext ); //If you get children in another choice then
				//both choices can have the same type of children.
				if ( childsInNext.getCount() == childsInChoice.getCount() ) 
				{
					//if there are the same ammount of children then all present children can belong to both
					//choices. Try to place the new child in this next choice.
					if ( _children[x]->placeElement( parent, child, ordinal, i, before, after ) != NULL ) 
					{
						retVal = child;
						ordinal = ordinal  + _ordinalOffset;

						myData->set( i, (daeChar)x ); //change the choice to this new one
						break;
					}
				}
			}
			if ( retVal != NULL ) break;
		}
	}
	return retVal;
}

daeBool daeMetaChoice::removeElement( daeElement *parent, daeElement *child ) {
	size_t cnt = _children.getCount();
	for ( size_t x = 0; x < cnt; x++ ) {
		if ( _children[x]->removeElement( parent, child ) ) {
			return true;
		}
	}
	return false;
}

daeMetaElement * daeMetaChoice::findChild( daeString elementName ) {
	daeMetaElement *me = NULL;
	size_t cnt = _children.getCount();
	for ( size_t x = 0; x < cnt; x++ ) {
		me = _children[x]->findChild( elementName );
		if ( me != NULL ) {
			return me;
		}
	}
	return NULL;
}

void daeMetaChoice::getChildren( daeElement *parent, daeElementRefArray &array ) {
	size_t cnt = _children.getCount();
	for ( size_t x = 0; x < cnt; x++ ) {
		_children[x]->getChildren( parent, array );
	}
}

