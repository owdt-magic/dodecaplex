#ifndef ATTRIBUTE_SYSTEM_H
#define ATTRIBUTE_SYSTEM_H

#include <utility>
#include <functional>

// Attribute system defines for improved readability
#define PARAMETER_NODE_BASE_ID 100
#define PARAMETER_ATTRIBUTE_BASE_ID 1000
#define VALUE_GENERATOR_NODE_ID 200
#define VALUE_GENERATOR_OUTPUT_ATTRIBUTE_ID 2001
#define AUDIO_BAND_ATTRIBUTE_MULTIPLIER 10
#define AUDIO_BAND_ATTRIBUTE_OFFSET 1
#define LINK_ID_MULTIPLIER 10000

#include "sharedUniforms.h"

// Helper functions for attribute system
namespace AttributeHelpers {
    // Convert parameter index to node ID
    inline int getParameterNodeId(int paramIndex) {
        return PARAMETER_NODE_BASE_ID + paramIndex;
    }
    
    // Convert parameter index to input attribute ID
    inline int getParameterAttributeId(int paramIndex) {
        return PARAMETER_ATTRIBUTE_BASE_ID + paramIndex;
    }
    
    // Convert parameter attribute ID back to parameter index
    inline int getParameterIndexFromAttributeId(int attributeId) {
        return attributeId - PARAMETER_ATTRIBUTE_BASE_ID;
    }
    
    // Convert audio band index to output attribute ID
    inline int getAudioBandAttributeId(int bandIndex) {
        return bandIndex * AUDIO_BAND_ATTRIBUTE_MULTIPLIER + AUDIO_BAND_ATTRIBUTE_OFFSET;
    }
    
    // Convert audio band attribute ID back to band index
    inline int getBandIndexFromAttributeId(int attributeId) {
        return attributeId / AUDIO_BAND_ATTRIBUTE_MULTIPLIER;
    }
    
    // Check if attribute ID is a parameter input
    inline bool isParameterAttribute(int attributeId) {
        return attributeId >= PARAMETER_ATTRIBUTE_BASE_ID && 
               attributeId < PARAMETER_ATTRIBUTE_BASE_ID + PARAM_COUNT;
    }
    
    // Check if attribute ID is an audio band output
    inline bool isAudioBandAttribute(int attributeId) {
        return attributeId % AUDIO_BAND_ATTRIBUTE_MULTIPLIER == AUDIO_BAND_ATTRIBUTE_OFFSET &&
               getBandIndexFromAttributeId(attributeId) >= 0 && 
               getBandIndexFromAttributeId(attributeId) < BAND_COUNT;
    }
    
    // Check if attribute ID is the value generator output
    inline bool isValueGeneratorAttribute(int attributeId) {
        return attributeId == VALUE_GENERATOR_OUTPUT_ATTRIBUTE_ID;
    }
    
    // Check if attribute ID is a valid output (audio band or value generator)
    inline bool isValidOutputAttribute(int attributeId) {
        return isAudioBandAttribute(attributeId) || isValueGeneratorAttribute(attributeId);
    }
    
    // Check if attribute ID is a valid input (parameter)
    inline bool isValidInputAttribute(int attributeId) {
        return isParameterAttribute(attributeId);
    }
    
    // Generate unique link ID
    inline int generateLinkId(int startAttr, int endAttr) {
        return startAttr * LINK_ID_MULTIPLIER + endAttr;
    }
}

// Lambda functions for conditional checks
namespace AttributePredicates {
    // Check if a link connects a specific audio band to a specific parameter
    inline auto isAudioBandToParameterLink(int bandIndex, int paramIndex) {
        return [bandIndex, paramIndex](const std::pair<int, int>& link) {
            return link.first == AttributeHelpers::getAudioBandAttributeId(bandIndex) && 
                   link.second == AttributeHelpers::getParameterAttributeId(paramIndex);
        };
    }
    
    // Check if a link connects the value generator to a specific parameter
    inline auto isValueGeneratorToParameterLink(int paramIndex) {
        return [paramIndex](const std::pair<int, int>& link) {
            return link.first == VALUE_GENERATOR_OUTPUT_ATTRIBUTE_ID && 
                   link.second == AttributeHelpers::getParameterAttributeId(paramIndex);
        };
    }
    
    // Check if a link connects a specific source to a specific destination
    inline auto isLinkBetweenAttributes(int startAttr, int endAttr) {
        return [startAttr, endAttr](const std::pair<int, int>& link) {
            return link.first == startAttr && link.second == endAttr;
        };
    }
    
    // Check if a link is from a valid output to a valid input
    inline auto isValidLink() {
        return [](const std::pair<int, int>& link) {
            return AttributeHelpers::isValidOutputAttribute(link.first) && 
                   AttributeHelpers::isValidInputAttribute(link.second);
        };
    }
}

#endif // ATTRIBUTE_SYSTEM_H 