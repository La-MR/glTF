// Copyright (c) 2012, Motorola Mobility, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of the Motorola Mobility, Inc. nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "GLTF.h"

using namespace rapidjson;
using namespace std::tr1;
using namespace std;

namespace GLTF 
{
    void GLTFMeshAttribute::_generateID() 
    {
        this->_ID = GLTFUtils::generateIDForType("attribute");
    }
    
    GLTFMeshAttribute::GLTFMeshAttribute(): _min(0), _max(0) 
    {
        this->setComponentType(NOT_AN_ELEMENT_TYPE);
        this->setByteStride(0);
        this->_byteOffset = 0;
        this->_generateID();
        this->setComponentsPerAttribute(0);
        this->setCount(0);
    }
        
    GLTFMeshAttribute::GLTFMeshAttribute(GLTFMeshAttribute* meshAttribute): 
    _bufferView(meshAttribute->getBufferView()), _min(0), _max(0)
    {
        assert(meshAttribute);
        
        this->setComponentType(meshAttribute->getComponentType());
        this->setByteStride(meshAttribute->getByteStride());
        this->_byteOffset = meshAttribute->getByteOffset();
        this->_generateID();
        this->setComponentsPerAttribute(meshAttribute->getComponentsPerAttribute());
        this->setCount(meshAttribute->getCount());
    }
        
    GLTFMeshAttribute::~GLTFMeshAttribute()
    {
        if (this->_min) {
            delete [] this->_min;
        }
        if (this->_max) {
            delete [] this->_max;
        }
    }
        
    void GLTFMeshAttribute::setBufferView(shared_ptr <GLTFBufferView> bufferView)
    {
        this->_bufferView = bufferView;
    }
        
    shared_ptr <GLTFBufferView> GLTFMeshAttribute::getBufferView()
    {
        return this->_bufferView;
    }
        
    void GLTFMeshAttribute::setComponentsPerAttribute(size_t componentsPerAttribute)
    {
        this->_componentsPerAttribute = componentsPerAttribute;
    }
        
    size_t GLTFMeshAttribute::getComponentsPerAttribute()
    {
        return this->_componentsPerAttribute;
    }
        
    void GLTFMeshAttribute::setByteStride(size_t byteStride)
    {
        this->_byteStride = byteStride;
    }
        
    size_t GLTFMeshAttribute::getByteStride()
    {
        return this->_byteStride;
    }
        
    void GLTFMeshAttribute::setComponentType(ComponentType componentType)
    {
        this->_componentType = componentType;
    }
        
    ComponentType GLTFMeshAttribute::getComponentType()
    {
        return this->_componentType;
    }
        
    void GLTFMeshAttribute::setByteOffset(size_t byteOffset)
    {
        this->_byteOffset = byteOffset;
    }
        
    size_t GLTFMeshAttribute::getByteOffset()
    {
        return this->_byteOffset;
    }
        
    size_t GLTFMeshAttribute::getCount()
    {
        return this->_count;
    }
    
    void GLTFMeshAttribute::setCount(size_t count)
    {
        this->_count = count;
    }
 
    const std::string& GLTFMeshAttribute::getID()
    {
        return this->_ID;
    }
        
    size_t GLTFMeshAttribute::getVertexAttributeByteLength()
    {
        size_t componentsPerAttribute = this->getComponentsPerAttribute();
        ComponentType type = this->getComponentType();
        switch (type) {
            case GLTF::BYTE:
            case GLTF::UNSIGNED_BYTE:
                return componentsPerAttribute;
            case GLTF::SHORT:
                return componentsPerAttribute * sizeof(short);                    
            case GLTF::UNSIGNED_SHORT:
                return componentsPerAttribute * sizeof(unsigned short);                    
            case GLTF::FIXED:
                return componentsPerAttribute * sizeof(int);                    
            case GLTF::FLOAT:
                return componentsPerAttribute * sizeof(float);
            default: 
                // FIXME: report error or just log ?
                break;
        }
        
        // FIXME: report error or just log ?
        return 0;
    }
    
    const double* GLTFMeshAttribute::getMin() {
        return this->_min;
    }

    const double* GLTFMeshAttribute::getMax() {
        return this->_max;
    }
    
    typedef struct {
        double *min, *max;
    } __MinMaxApplierInfo;
    
    static void __ComputeMinMax(void *value,
                               ComponentType type,
                               size_t componentsPerAttribute,
                               size_t index,
                               size_t vertexAttributeByteSize,
                               void *context) {
        __MinMaxApplierInfo *applierInfo = (__MinMaxApplierInfo*)context;
        char* bufferData = (char*)value;

        switch (type) {
            case GLTF::FLOAT: {
                float* vector = (float*)bufferData;
                for (size_t j = 0 ; j < componentsPerAttribute ; j++) {
                    float value = vector[j];
                    if (value < applierInfo->min[j]) {
                        applierInfo->min[j] = value;
                    }
                    if (value > applierInfo->max[j]) {
                        applierInfo->max[j] = value;
                    }
                }
            }
                break;
            default:
                break;
        }
    }
    
    void GLTFMeshAttribute::computeMinMax() 
    {
        //size_t byteStride = this->getByteStride();
        size_t componentsPerAttribute = this->getComponentsPerAttribute();

        //realloc
        if (this->_min) {
            delete [] this->_min;
        }
        if (this->_max) {
            delete [] this->_max;
        }
        this->_min = new double[componentsPerAttribute];
        this->_max = new double[componentsPerAttribute];

        __MinMaxApplierInfo minMaxApplierInfo;
        minMaxApplierInfo.min = this->_min;
        minMaxApplierInfo.max = this->_max;
        
        for (size_t i = 0 ; i < componentsPerAttribute ; i++) {
            this->_min[i] = DBL_MAX;
            this->_max[i] = -DBL_MAX;
        }
        
        apply(__ComputeMinMax, &minMaxApplierInfo);
    }
    
    void GLTFMeshAttribute::apply(GLTFMeshAttributeApplierFunc applierFunc, void* context)
    {
        size_t byteStride = this->getByteStride();
        size_t componentsPerAttribute = this->getComponentsPerAttribute();
        size_t vertexAttributeByteSize = this->getVertexAttributeByteLength();
        shared_ptr <GLTF::GLTFBufferView> bufferView = this->getBufferView();
        ComponentType type = this->getComponentType();
        unsigned char* bufferData = (unsigned char*)bufferView->getBufferDataByApplyingOffset();

        for (size_t i = 0 ; i < _count ; i++) {
            (*applierFunc)(bufferData + (i * byteStride), type, componentsPerAttribute, i, vertexAttributeByteSize, context);
        }
    }
    
    bool GLTFMeshAttribute::matchesLayout(GLTFMeshAttribute* meshAttribute)
    {
        assert(meshAttribute);
        
        return ((meshAttribute->getComponentsPerAttribute() == this->getComponentsPerAttribute()) &&
                (meshAttribute->getByteStride() == this->getByteStride()) &&
                (meshAttribute->getComponentType() == this->getComponentType()) &&
                (meshAttribute->getVertexAttributeByteLength() == this->getVertexAttributeByteLength()));
    }
    
    std::string GLTFMeshAttribute::getGLType() {
        //FIXME: simplified version 
        //FIXME: at the moment our ComponentType does not support Mat. We will probably have to add it for Skinning
        //OpenGL ES2.0 supports GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, or GL_FLOAT_MAT4
        return GLTFUtils::getStringForGLType(this->_componentType) + "_VEC" + GLTFUtils::toString(this->_componentsPerAttribute);
    }

}
