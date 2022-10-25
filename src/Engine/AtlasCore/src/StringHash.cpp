#include "AtlasCorePCH.h"
#include "StringHash.h"

atlas::core::StringHashView::StringHashView(const StringHash& value)
    : m_StringHash{value.m_StringHash}
    , m_String{value.m_String}
{
}
