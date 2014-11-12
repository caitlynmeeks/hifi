//
//  BoxEntityItem.cpp
//  libraries/entities/src
//
//  Created by Brad Hefta-Gaub on 12/4/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include <QDebug>

#include <ByteCountCoding.h>

#include "BoxEntityItem.h"
#include "EntityMotionState.h"
#include "EntityTree.h"
#include "EntityTreeElement.h"


EntityItem* BoxEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    EntityItem* result = new BoxEntityItem(entityID, properties);
    return result;
}

BoxEntityItem::BoxEntityItem(const EntityItemID& entityItemID, const EntityItemProperties& properties) :
        EntityItem(entityItemID) 
{
    _type = EntityTypes::Box;
    _created = properties.getCreated();
    setProperties(properties, true);
}

EntityItemProperties BoxEntityItem::getProperties() const {
    EntityItemProperties properties = EntityItem::getProperties(); // get the properties from our base class

    properties._color = getXColor();
    properties._colorChanged = false;

    properties._glowLevel = getGlowLevel();
    properties._glowLevelChanged = false;

    return properties;
}

bool BoxEntityItem::setProperties(const EntityItemProperties& properties, bool forceCopy) {
    bool somethingChanged = false;
    somethingChanged = EntityItem::setProperties(properties, forceCopy); // set the properties in our base class

    SET_ENTITY_PROPERTY_FROM_PROPERTIES(color, setColor);

    if (somethingChanged) {
        bool wantDebug = false;
        if (wantDebug) {
            uint64_t now = usecTimestampNow();
            int elapsed = now - getLastEdited();
            qDebug() << "BoxEntityItem::setProperties() AFTER update... edited AGO=" << elapsed <<
                    "now=" << now << " getLastEdited()=" << getLastEdited();
        }
        setLastEdited(properties._lastEdited);
    }
    return somethingChanged;
}

int BoxEntityItem::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead, 
                                                ReadBitstreamToTreeParams& args,
                                                EntityPropertyFlags& propertyFlags, bool overwriteLocalData) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY_COLOR(PROP_COLOR, _color);

    return bytesRead;
}


// TODO: eventually only include properties changed since the params.lastViewFrustumSent time
EntityPropertyFlags BoxEntityItem::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties = EntityItem::getEntityProperties(params);
    requestedProperties += PROP_COLOR;
    return requestedProperties;
}

void BoxEntityItem::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params, 
                                    EntityTreeElementExtraEncodeData* modelTreeElementExtraEncodeData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount, 
                                    OctreeElement::AppendState& appendState) const { 

    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_COLOR, appendColor, getColor());
}

#ifdef USE_BULLET_PHYSICS
// BOOKMARK: move EntityMotionState implementation to Entities
// also define interface for it (dunno what to call it)
EntityMotionState* BoxEntityItem::createMotionState() {
    if (!_motionState) {
        _motionState = new EntityMotionState(this);
        glm::vec3 extents = getDimensionsInMeters();
        _motionState->setVolume(extents.x * extents.y * extents.z);
    }
    return _motionState;
}
#endif // USE_BULLET_PHYSICS

