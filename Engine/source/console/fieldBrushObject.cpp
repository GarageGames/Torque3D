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

#include "core/strings/stringUnit.h"
#include "console/fieldBrushObject.h"
#include "console/engineAPI.h"

// Prefix added to dynamic-fields when they're used to store any copied static-fields when peristing.
#define INTERNAL_FIELD_PREFIX       "_fieldBrush_"

// Size of return buffers used.
const U32 bufferSizes = 1024 * 4;


IMPLEMENT_CONOBJECT(FieldBrushObject);

ConsoleDocClass( FieldBrushObject,
   "@brief For static-field copying/pasting, editor use only\n\n"
   "@internal"
);

FieldBrushObject::FieldBrushObject()
{
    // Reset Description.
    mDescription = StringTable->EmptyString();
    mSortName    = StringTable->EmptyString();
}


//-----------------------------------------------------------------------------
// Persist Fields.
//-----------------------------------------------------------------------------
void FieldBrushObject::initPersistFields()
{
    // Add Fields.
    addProtectedField("description", TypeCaseString, Offset(mDescription, FieldBrushObject), setDescription, defaultProtectedGetFn, "");
    addProtectedField("sortName", TypeString, Offset(mSortName, FieldBrushObject), setSortName, defaultProtectedGetFn, "");

    // Call Parent.
    Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// Remove from Sim.
//-----------------------------------------------------------------------------
void FieldBrushObject::onRemove()
{
    // Destroy any fields.
    destroyFields();

    // Call Parent.
    Parent::onRemove();
}


//-----------------------------------------------------------------------------
// Destroy any fields.
//-----------------------------------------------------------------------------
void FieldBrushObject::destroyFields()
{
    // Fetch Dynamic-Field Dictionary.
    SimFieldDictionary* pFieldDictionary = getFieldDictionary();

    // Any Field Dictionary?
    if ( pFieldDictionary == NULL )
    {
        // No, so we're done.
        return;
    }

    // Iterate fields.
    for ( SimFieldDictionaryIterator itr(pFieldDictionary); *itr; ++itr )
    {
        // Fetch Field Entry.
        SimFieldDictionary::Entry* fieldEntry = *itr;

        // Internal Field?
        if ( dStrstr( fieldEntry->slotName, INTERNAL_FIELD_PREFIX ) == fieldEntry->slotName )
        {
            // Yes, so remove it.
            pFieldDictionary->setFieldValue( fieldEntry->slotName, "" );
        }
    }
}


//-----------------------------------------------------------------------------
// Suppress Spaces.
//-----------------------------------------------------------------------------
static char replacebuf[1024];
static char* suppressSpaces(const char* in_pname)
{
   U32 i = 0;
   char chr;
   do
   {
      chr = in_pname[i];
      replacebuf[i++] = (chr != 32) ? chr : '_';
   } while(chr);

   return replacebuf;
}

//-----------------------------------------------------------------------------
// Query Groups.
//-----------------------------------------------------------------------------
DefineConsoleMethod(FieldBrushObject, queryGroups, const char*, (const char* simObjName), , "(simObject) Query available static-field groups for selected object./\n"
                                                                "@param simObject Object to query static-field groups on.\n"
                                                             "@return Space-seperated static-field group list.")
{
    // Fetch selected object.
    SimObject* pSimObject = dynamic_cast<SimObject*>( Sim::findObject( simObjName ) );

    // Valid object?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf("FieldBrushObject::queryFieldGroups() - Invalid SimObject!");
        return NULL;
    }

    // Create Returnable Buffer.
    S32 maxBuffer = bufferSizes;
    char* pReturnBuffer = Con::getReturnBuffer(bufferSizes);
    char* pBuffer = pReturnBuffer;

    // Fetch Field List.
    const AbstractClassRep::FieldList& staticFields = pSimObject->getFieldList();

    // Iterate Fields.
    for( U32 fieldIndex = 0; fieldIndex < staticFields.size(); ++fieldIndex )
    {
        // Fetch Field.
        const AbstractClassRep::Field& staticField = staticFields[fieldIndex];

        // Start Group?
        if ( staticField.type == AbstractClassRep::StartGroupFieldType )
        {
            // Yes, so write-out group-name without spaces...
            char* pGroupNameNoSpaces = suppressSpaces(staticField.pGroupname);

            // Will the field fit?
            // NOTE:-   We used "-1" to include the suffix space.
            if ( (maxBuffer - (S32)dStrlen(pGroupNameNoSpaces) - 1) >= 0 )
            {
                // Yes...
                // NOTE:-   The group-name does not have the "_begingroup" suffix which should stay hidden.
                S32 charsWritten = dSprintf( pBuffer, maxBuffer, "%s ", pGroupNameNoSpaces );
                pBuffer += charsWritten;
                maxBuffer -= charsWritten;
            }
            else
            {
                // No, so warn.
                Con::warnf("FieldBrushObject::queryGroups() - Couldn't fit all groups into return string!");
                break;
            }
        }
    }

    // Strip final space.
    if ( pBuffer != pReturnBuffer )
    {
        *(pBuffer-1) = 0;
    }

    // Return Buffer.
    return pReturnBuffer;
}


//-----------------------------------------------------------------------------
// Query Fields.
//-----------------------------------------------------------------------------
DefineConsoleMethod(FieldBrushObject, queryFields, const char*, (const char* simObjName, const char* groupList), (""), "(simObject, [groupList]) Query available static-fields for selected object./\n"
                                                                "@param simObject Object to query static-fields on.\n"
                                                                "@param groupList groups to filter static-fields against.\n"
                                                             "@return Space-seperated static-field list.")
{
    // Fetch selected object.
    SimObject* pSimObject = dynamic_cast<SimObject*>( Sim::findObject( simObjName ) );

    // Valid object?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf("FieldBrushObject::queryFields() - Invalid SimObject!");
        return NULL;
    }

    // Create Returnable Buffer.
    S32 maxBuffer = bufferSizes;
    char* pReturnBuffer = Con::getReturnBuffer(bufferSizes);
    char* pBuffer = pReturnBuffer;

    // Fetch Field List.
    const AbstractClassRep::FieldList& staticFields = pSimObject->getFieldList();

    // Did we specify a groups list?
    if ( String::isEmpty(groupList) )
    {
        // No, so return all fields...

        // Iterate fields.
        for( U32 fieldIndex = 0; fieldIndex < staticFields.size(); ++fieldIndex )
        {
            // Fetch Field.
            const AbstractClassRep::Field& staticField = staticFields[fieldIndex];

            // Standard Field?
            if (    staticField.type != AbstractClassRep::StartGroupFieldType &&
                    staticField.type != AbstractClassRep::EndGroupFieldType &&
                    staticField.type != AbstractClassRep::DeprecatedFieldType )
            {
                // Yes, so will the field fit?
                // NOTE:-   We used "-1" to include the suffix space.
                if ( (maxBuffer - (S32)dStrlen(staticField.pFieldname) - 1) >= 0 )
                {
                    // Yes, so write-out field-name.
                    S32 charsWritten = dSprintf( pBuffer, maxBuffer, "%s ", staticField.pFieldname );
                    pBuffer += charsWritten;
                    maxBuffer -= charsWritten;
                }
                else
                {
                    // No, so warn.
                    Con::warnf("FieldBrushObject::queryFields() - Couldn't fit all fields into return string!");
                    break;
                }
            }
        }

        // Strip final space.
        if ( pBuffer != pReturnBuffer )
        {
            *(pBuffer-1) = 0;
        }

        // Return field list.
        return pReturnBuffer;
    }

    // Yes, so filter by groups...

    // Group List.
    Vector<StringTableEntry> groups;
    // Yes, so fetch group list.
    // Yes, so calculate group Count.
    const U32 groupCount = StringUnit::getUnitCount( groupList, " \t\n" );

    char tempBuf[256];

    // Iterate groups...
    for ( U32 groupIndex = 0; groupIndex < groupCount; ++groupIndex )
    {
        // Copy string element.
        dStrcpy( tempBuf, StringUnit::getUnit( groupList, groupIndex, " \t\n" ) );
        // Append internal name.
        dStrcat( tempBuf, "_begingroup" );
        // Store Group.
        groups.push_back( StringTable->insert( tempBuf ) );
    }

    // Reset Valid Group.
    bool validGroup = false;

    // Iterate fields.
    for( U32 fieldIndex = 0; fieldIndex < staticFields.size(); ++fieldIndex )
    {
        // Fetch Field.
        const AbstractClassRep::Field& staticField = staticFields[fieldIndex];

        // Handle Group Type.
        switch( staticField.type )
        {
            // Start Group.
            case AbstractClassRep::StartGroupFieldType:
            {
                // Is this group valid?

                // Iterate groups...
                for ( U32 groupIndex = 0; groupIndex < groups.size(); ++groupIndex )
                {
                    // Group selected?
                    if ( groups[groupIndex] == staticField.pFieldname )
                    {
                        // Yes, so flag as valid.
                        validGroup = true;
                        break;
                    }
                }

            } break;

            // End Group.
            case AbstractClassRep::EndGroupFieldType:
            {
                // Reset Valid Group.
                validGroup = false;

            } break;

            // Deprecated.
            case AbstractClassRep::DeprecatedFieldType:
            {
            } break;

            // Standard.
            default:
            {
                // Do we have a valid group?
                if ( validGroup )
                {
                    // Yes, so will the field fit?
                    // NOTE:-   We used "-1" to include the suffix space.
                    if ( (maxBuffer - (S32)dStrlen(staticField.pFieldname) - 1) >= 0 )
                    {
                        // Yes, so write-out field-name.
                        S32 charsWritten = dSprintf( pBuffer, maxBuffer, "%s ", staticField.pFieldname );
                        pBuffer += charsWritten;
                        maxBuffer -= charsWritten;
                    }
                    else
                    {
                        // No, so warn.
                        Con::warnf("FieldBrushObject::queryFields() - Couldn't fit all fields into return string!");
                        // HACK: Easy way to finish iterating fields.
                        fieldIndex = staticFields.size();
                        break;
                    }
                }

            } break;
        };
    }

    // Strip final space.
    if ( pBuffer != pReturnBuffer )
    {
        *(pBuffer-1) = 0;
    }

    // Return field list.
    return pReturnBuffer;
}

//-----------------------------------------------------------------------------
// Copy Fields.
//-----------------------------------------------------------------------------
DefineConsoleMethod(FieldBrushObject, copyFields, void, (const char* simObjName, const char* pFieldList), (""), "(simObject, [fieldList]) Copy selected static-fields for selected object./\n"
                                                        "@param simObject Object to copy static-fields from.\n"
                                                        "@param fieldList fields to filter static-fields against.\n"
                                                     "@return No return value.")
{
    // Fetch selected object.
    SimObject* pSimObject = dynamic_cast<SimObject*>( Sim::findObject( simObjName ) );

    // Valid object?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf("FieldBrushObject::copyFields() - Invalid SimObject!");
        return;
    }

    // Fetch field list.
    
    // Copy Fields.
    object->copyFields( pSimObject, pFieldList );
}
// Copy Fields.
void FieldBrushObject::copyFields( SimObject* pSimObject, const char* fieldList )
{
    // FieldBrushObject class?   
    if ( dStrcmp(pSimObject->getClassName(), getClassName()) == 0 )
    {
        // Yes, so warn.
        Con::warnf("FieldBrushObject::copyFields() - Cannot copy FieldBrushObject objects!");
        return;
    }

    char tempBuf[bufferSizes];

    // Field List.
    Vector<StringTableEntry> fields;

    // Fetch valid field-list flag.
    bool validFieldList = ( fieldList != NULL );

    // Did we specify a fields list?
    if ( validFieldList )
    {
        // Yes, so calculate field Count.
        const U32 fieldCount = StringUnit::getUnitCount( fieldList, " \t\n" );

        // Iterate fields...
        for ( U32 fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex )
        {
            // Copy string element.
            dStrcpy( tempBuf, StringUnit::getUnit( fieldList, fieldIndex, " \t\n" ) );

            // Store field.
            fields.push_back( StringTable->insert( tempBuf ) );
        }
    }

    // Destroy Fields.
    destroyFields();

    // Fetch Field List.
    const AbstractClassRep::FieldList& staticFields = pSimObject->getFieldList();

    // Iterate fields.
    for( U32 fieldIndex = 0; fieldIndex < staticFields.size(); ++fieldIndex )
    {
        // Fetch Field.
        const AbstractClassRep::Field& staticField = staticFields[fieldIndex];

        // Standard Field?
        if (    staticField.type != AbstractClassRep::StartGroupFieldType &&
                staticField.type != AbstractClassRep::EndGroupFieldType &&
                staticField.type != AbstractClassRep::DeprecatedFieldType )
        {
            // Set field-specified flag.
            bool fieldSpecified = !validFieldList;

            // Did we specify a fields list?
            if ( validFieldList )
            {
                // Yes, so is this field name selected?

                // Iterate fields...
                for ( U32 fieldIndex = 0; fieldIndex < fields.size(); ++fieldIndex )
                {
                    // Field selected?
                    if ( staticField.pFieldname == fields[fieldIndex] )
                    {
                        // Yes, so flag as such.
                        fieldSpecified = true;
                        break;
                    }
                }
            }

            // Field specified?
            if ( fieldSpecified )
            {
                if ( staticField.elementCount <= 1 )
                {
                    for( U32 fieldElement = 0; S32(fieldElement) < staticField.elementCount; ++fieldElement )
                    {
                        // Fetch Field Value.
                        const char* fieldValue = (staticField.getDataFn)( pSimObject, Con::getData(staticField.type, (void *) (((const char *)pSimObject) + staticField.offset), fieldElement, staticField.table, staticField.flag) );

                        // Field Value?
                        if ( fieldValue )
                        {
                            // Yes.
                            dSprintf( tempBuf, sizeof(tempBuf), INTERNAL_FIELD_PREFIX"%s", staticField.pFieldname );

                            // Fetch Dynamic-Field Dictionary.
                            SimFieldDictionary* pFieldDictionary = getFieldDictionary();

                            // Set field value.
                            if ( !pFieldDictionary )
                            {
                                setDataField( StringTable->insert( tempBuf ), NULL, fieldValue );
                            }
                            else
                            {
                                pFieldDictionary->setFieldValue( StringTable->insert( tempBuf ), fieldValue );
                            }
                        }
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Paste Fields.
//-----------------------------------------------------------------------------
DefineConsoleMethod(FieldBrushObject, pasteFields, void, (const char* simObjName), , "(simObject) Paste copied static-fields to selected object./\n"
                                                        "@param simObject Object to paste static-fields to.\n"
                                                     "@return No return value.")
{
    // Fetch selected object.
    SimObject* pSimObject = dynamic_cast<SimObject*>( Sim::findObject( simObjName ) );

    // Valid object?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf("FieldBrushObject::pasteFields() - Invalid SimObject!");
        return;
    }

    // Paste Fields.
    object->pasteFields( pSimObject );
}
// Paste Fields.
void FieldBrushObject::pasteFields( SimObject* pSimObject )
{
    // FieldBrushObject class?   
    if ( dStrcmp(pSimObject->getClassName(), getClassName()) == 0 )
    {
        // Yes, so warn.
        Con::warnf("FieldBrushObject::pasteFields() - Cannot paste FieldBrushObject objects!");
        return;
    }

    // Fetch Dynamic-Field Dictionary.
    SimFieldDictionary* pFieldDictionary = getFieldDictionary();

    // Any Field Dictionary?
    if ( pFieldDictionary == NULL )
    {
        // No, so we're done.
        return;
    }

    // Force modification of static-fields on target object!
    pSimObject->setModStaticFields( true );

    // Iterate fields.
    for ( SimFieldDictionaryIterator itr(pFieldDictionary); *itr; ++itr )
    {
        // Fetch Field Entry.
        SimFieldDictionary::Entry* fieldEntry = *itr;

        // Internal Field?
        char* pInternalField = dStrstr( fieldEntry->slotName, INTERNAL_FIELD_PREFIX );
        if ( pInternalField == fieldEntry->slotName )
        {
            // Yes, so skip the prefix.
            pInternalField += dStrlen(INTERNAL_FIELD_PREFIX);

            // Is this a static-field on the target object?
            // NOTE:-   We're doing this so we don't end-up creating a dynamic-field if it isn't present.

            // Fetch Field List.
            const AbstractClassRep::FieldList& staticFields = pSimObject->getFieldList();

            // Iterate fields.
            for( U32 fieldIndex = 0; fieldIndex < staticFields.size(); ++fieldIndex )
            {
                // Fetch Field.
                const AbstractClassRep::Field& staticField = staticFields[fieldIndex];

                // Standard Field?
                if (    staticField.type != AbstractClassRep::StartGroupFieldType &&
                        staticField.type != AbstractClassRep::EndGroupFieldType &&
                        staticField.type != AbstractClassRep::DeprecatedFieldType )
                {
                    // Target field?
                    if ( dStrcmp(staticField.pFieldname, pInternalField) == 0 )
                    {
                        // Yes, so set data.
                        pSimObject->setDataField( staticField.pFieldname, NULL, fieldEntry->value );
                    }
                }
            }
        }
    }
}
