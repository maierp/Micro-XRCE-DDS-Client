// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*! 
 * @file ShapeType.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _ShapeType_H_
#define _ShapeType_H_

#include <stdint.h>
#include <stdbool.h>

/*!
 * @brief This struct represents the structure ShapeType defined by the user in the IDL file.
 * @ingroup SHAPETYPE
 */
typedef struct ShapeType
{
    char* color;
    int32_t x;
    int32_t y;
    int32_t shapesize;

} ShapeType;

typedef struct MicroBuffer MicroBuffer;

bool serialize_ShapeType_topic(MicroBuffer* writer, const ShapeType* topic);
bool deserialize_ShapeType_topic(MicroBuffer* reader, ShapeType* topic);
uint32_t size_of_ShapeType_topic(const ShapeType* topic, uint32_t size);

#endif // _ShapeType_H_