//
// Copyright 2017 Animal Logic
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.//
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "AL/maya/CommandGuiHelper.h"
#include "AL/maya/Common.h"
#include "AL/usdmaya/AttributeType.h"
#include "AL/usdmaya/TypeIDs.h"
#include "AL/usdmaya/Utils.h"
#include "AL/usdmaya/DebugCodes.h"
#include "AL/usdmaya/nodes/Transform.h"
#include "AL/usdmaya/nodes/TransformationMatrix.h"

#include "maya/MFileIO.h"
#include "maya/MGlobal.h"

namespace AL {
namespace usdmaya {
namespace nodes {

//----------------------------------------------------------------------------------------------------------------------
const MTypeId TransformationMatrix::kTypeId(AL_USDMAYA_TRANSFORMATION_MATRIX);

//----------------------------------------------------------------------------------------------------------------------
MPxTransformationMatrix* TransformationMatrix::creator()
{
  return new TransformationMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
TransformationMatrix::TransformationMatrix()
  : MPxTransformationMatrix(),
    m_prim(),
    m_xform(),
    m_time(UsdTimeCode::Default()),
    m_scaleTweak(0, 0, 0),
    m_rotationTweak(0, 0, 0),
    m_translationTweak(0, 0, 0),
    m_shearTweak(0, 0, 0),
    m_scalePivotTweak(0, 0, 0),
    m_scalePivotTranslationTweak(0, 0, 0),
    m_rotatePivotTweak(0, 0, 0),
    m_rotatePivotTranslationTweak(0, 0, 0),
    m_rotateOrientationTweak(0, 0, 0, 1.0),
    m_scaleFromUsd(1.1, 1.1, 1.1),
    m_rotationFromUsd(5, 0, 0),
    m_translationFromUsd(0.1, 0.2, 0.3),
    m_shearFromUsd(0, 0, 0),
    m_scalePivotFromUsd(0, 0, 0),
    m_scalePivotTranslationFromUsd(0, 0, 0),
    m_rotatePivotFromUsd(0, 0, 0),
    m_rotatePivotTranslationFromUsd(0, 0, 0),
    m_rotateOrientationFromUsd(0, 0, 0, 1.0),
    m_localTranslateOffset(0, 0, 0),
    m_flags(0)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::TransformationMatrix\n");
  initialiseToPrim();
}

//----------------------------------------------------------------------------------------------------------------------
TransformationMatrix::TransformationMatrix(const UsdPrim& prim)
  : MPxTransformationMatrix(),
    m_prim(prim),
    m_xform(prim),
    m_time(UsdTimeCode::Default()),
    m_scaleTweak(0, 0, 0),
    m_rotationTweak(0, 0, 0),
    m_translationTweak(0, 0, 0),
    m_shearTweak(0, 0, 0),
    m_scalePivotTweak(0, 0, 0),
    m_scalePivotTranslationTweak(0, 0, 0),
    m_rotatePivotTweak(0, 0, 0),
    m_rotatePivotTranslationTweak(0, 0, 0),
    m_rotateOrientationTweak(0, 0, 0, 1.0),

    m_scaleFromUsd(1.0, 1.0, 1.0),
    m_rotationFromUsd(0, 0, 0),
    m_translationFromUsd(0, 0, 0),
    m_shearFromUsd(0, 0, 0),
    m_scalePivotFromUsd(0, 0, 0),
    m_scalePivotTranslationFromUsd(0, 0, 0),
    m_rotatePivotFromUsd(0, 0, 0),
    m_rotatePivotTranslationFromUsd(0, 0, 0),
    m_rotateOrientationFromUsd(0, 0, 0, 1.0),
    m_localTranslateOffset(0, 0, 0),
    m_flags(0)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::TransformationMatrix\n");
  initialiseToPrim();
}

//----------------------------------------------------------------------------------------------------------------------
void TransformationMatrix::setPrim(const UsdPrim& prim)
{
  if(prim.IsValid())
  {
    TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setPrim %s\n", prim.GetName().GetText());
    m_prim = prim;
    UsdGeomXform xform(prim);
    m_xform = xform;
  }
  else
  {
    TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setPrim null\n");
    m_prim = UsdPrim();
    m_xform = UsdGeomXform();
  }
  m_time = UsdTimeCode(UsdTimeCode::Default());
  m_flags = 0;
  m_scaleTweak = MVector(0, 0, 0);
  m_rotationTweak = MEulerRotation(0, 0, 0);
  m_translationTweak = MVector(0, 0, 0);
  m_shearTweak = MVector(0, 0, 0);
  m_scalePivotTweak = MPoint(0, 0, 0);
  m_scalePivotTranslationTweak = MVector(0, 0, 0);
  m_rotatePivotTweak = MPoint(0, 0, 0);
  m_rotatePivotTranslationTweak = MVector(0, 0, 0);
  m_rotateOrientationTweak = MQuaternion(0, 0, 0, 1.0);
  m_localTranslateOffset = MVector(0, 0, 0);

  if(m_prim.IsValid())
  {
    m_scaleFromUsd = MVector(1.0, 1.0, 1.0);
    m_rotationFromUsd = MEulerRotation(0, 0, 0);
    m_translationFromUsd = MVector(0, 0, 0);
    m_shearFromUsd = MVector(0, 0, 0);
    m_scalePivotFromUsd = MPoint(0, 0, 0);
    m_scalePivotTranslationFromUsd = MVector(0, 0, 0);
    m_rotatePivotFromUsd = MPoint(0, 0, 0);
    m_rotatePivotTranslationFromUsd = MVector(0, 0, 0);
    m_rotateOrientationFromUsd = MQuaternion(0, 0, 0, 1.0);
    //if(MFileIO::isOpeningFile())
    {
      initialiseToPrim(!MFileIO::isOpeningFile());
    }
  }

  MPxTransformationMatrix::scaleValue = m_scaleFromUsd;
  MPxTransformationMatrix::rotationValue = m_rotationFromUsd;
  MPxTransformationMatrix::translationValue = m_translationFromUsd;
  MPxTransformationMatrix::shearValue = m_shearFromUsd;
  MPxTransformationMatrix::scalePivotValue = m_scalePivotFromUsd;
  MPxTransformationMatrix::scalePivotTranslationValue = m_scalePivotTranslationFromUsd;
  MPxTransformationMatrix::rotatePivotValue = m_rotatePivotFromUsd;
  MPxTransformationMatrix::rotatePivotTranslationValue = m_rotatePivotTranslationFromUsd;
  MPxTransformationMatrix::rotateOrientationValue = m_rotateOrientationFromUsd;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::readVector(MVector& result, const UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);
  switch(attr_type)
  {
  case UsdDataType::kVec3d:
    {
      GfVec3d value;
      const bool retValue = op.GetAs<GfVec3d>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = value[0];
      result.y = value[1];
      result.z = value[2];
    }
    break;

  case UsdDataType::kVec3f:
    {
      GfVec3f value;
      const bool retValue = op.GetAs<GfVec3f>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = double(value[0]);
      result.y = double(value[1]);
      result.z = double(value[2]);
    }
    break;

  case UsdDataType::kVec3h:
    {
      GfVec3h value;
      const bool retValue = op.GetAs<GfVec3h>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = double(value[0]);
      result.y = double(value[1]);
      result.z = double(value[2]);
    }
    break;

  case UsdDataType::kVec3i:
    {
      GfVec3i value;
      const bool retValue = op.GetAs<GfVec3i>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = double(value[0]);
      result.y = double(value[1]);
      result.z = double(value[2]);
    }
    break;

  default:
    return false;
  }

  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::readVector %f %f %f\n%s\n", result.x, result.y, result.z, op.GetOpName().GetText());
  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::pushVector(const MVector& result, UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::pushVector %f %f %f\n%s\n", result.x, result.y, result.z, op.GetOpName().GetText());
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);

  switch(attr_type)
  {
  case UsdDataType::kVec3d:
    {
      GfVec3d value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  case UsdDataType::kVec3f:
    {
      GfVec3f value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  case UsdDataType::kVec3h:
    {
      GfVec3h value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  case UsdDataType::kVec3i:
    {
      GfVec3i value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  default:
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::pushShear(const MVector& result, UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::pushShear %f %f %f\n%s\n", result.x, result.y, result.z, op.GetOpName().GetText());
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);
  switch(attr_type)
  {
  case UsdDataType::kMatrix4d:
    {
      GfMatrix4d m(
          1.0,      0.0,      0.0, 0.0,
          result.x, 1.0,      0.0, 0.0,
          result.y, result.z, 1.0, 0.0,
          0.0,      0.0,      0.0, 1.0);
      op.Set(m, timeCode);
    }
    break;

  default:
    return false;
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::readShear(MVector& result, const UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);
  switch(attr_type)
  {
  case UsdDataType::kMatrix4d:
    {
      GfMatrix4d value;
      const bool retValue = op.GetAs<GfMatrix4d>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = value[1][0];
      result.y = value[2][0];
      result.z = value[2][1];
    }
    break;

  default:
    return false;
  }
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::readShear %f %f %f\n%s\n", result.x, result.y, result.z, op.GetOpName().GetText());
  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::readPoint(MPoint& result, const UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);
  switch(attr_type)
  {
  case UsdDataType::kVec3d:
    {
      GfVec3d value;
      const bool retValue = op.GetAs<GfVec3d>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = value[0];
      result.y = value[1];
      result.z = value[2];
    }
    break;

  case UsdDataType::kVec3f:
    {
      GfVec3f value;
      const bool retValue = op.GetAs<GfVec3f>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = double(value[0]);
      result.y = double(value[1]);
      result.z = double(value[2]);
    }
    break;

  case UsdDataType::kVec3h:
    {
      GfVec3h value;
      const bool retValue = op.GetAs<GfVec3h>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = double(value[0]);
      result.y = double(value[1]);
      result.z = double(value[2]);
    }
    break;

  case UsdDataType::kVec3i:
    {
      GfVec3i value;
      const bool retValue = op.GetAs<GfVec3i>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result.x = double(value[0]);
      result.y = double(value[1]);
      result.z = double(value[2]);
    }
    break;

  default:
    return false;
  }
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::readPoint %f %f %f\n%s\n", result.x, result.y, result.z, op.GetOpName().GetText());

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::readMatrix(MMatrix& result, const UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::readMatrix\n");
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);
  switch(attr_type)
  {
  case UsdDataType::kMatrix4d:
    {
      GfMatrix4d value;
      const bool retValue = op.GetAs<GfMatrix4d>(&value, timeCode);
      if (!retValue)
      {
        return false;
      }
      result = *(const MMatrix*)(&value);
    }
    break;

  default:
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::pushMatrix(const MMatrix& result, UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::pushMatrix\n");
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);
  switch(attr_type)
  {
  case UsdDataType::kMatrix4d:
    {
      GfMatrix4d value = *(const GfMatrix4d*)(&result);
      const bool retValue = op.Set<GfMatrix4d>(value, timeCode);
      if (!retValue)
      {
        return false;
      }
    }
    break;

  default:
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::pushPoint(const MPoint& result, UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::pushPoint %f %f %f\n%s\n", result.x, result.y, result.z, op.GetOpName().GetText());
  const SdfValueTypeName vtn = op.GetTypeName();
  UsdDataType attr_type = getAttributeType(vtn);
  switch(attr_type)
  {
  case UsdDataType::kVec3d:
    {
      GfVec3d value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  case UsdDataType::kVec3f:
    {
      GfVec3f value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  case UsdDataType::kVec3h:
    {
      GfVec3h value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  case UsdDataType::kVec3i:
    {
      GfVec3i value(result.x, result.y, result.z);
      op.Set(value, timeCode);
    }
    break;

  default:
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
double TransformationMatrix::readDouble(const UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  double result = 0;
  UsdDataType attr_type = getAttributeType(op.GetTypeName());
  switch(attr_type)
  {
  case UsdDataType::kHalf:
    {
      GfHalf value;
      const bool retValue = op.Get<GfHalf>(&value, timeCode);
      if (retValue)
      {
        result = float(value);
      }
    }
    break;

  case UsdDataType::kFloat:
    {
      float value;
      const bool retValue = op.Get<float>(&value, timeCode);
      if (retValue)
      {
        result = double(value);
      }
    }
    break;

  case UsdDataType::kDouble:
    {
      double value;
      const bool retValue = op.Get<double>(&value, timeCode);
      if (retValue)
      {
        result = value;
      }
    }
    break;

  case UsdDataType::kInt:
    {
      int32_t value;
      const bool retValue = op.Get<int32_t>(&value, timeCode);
      if (retValue)
      {
        result = double(value);
      }
    }
    break;

  default:
    break;
  }
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::readDouble %d\n%s\n", result, op.GetOpName().GetText());
  return result;
}

//----------------------------------------------------------------------------------------------------------------------
void TransformationMatrix::pushDouble(const double value, UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::pushDouble %d\n%s\n", value, op.GetOpName().GetText());
  UsdDataType attr_type = getAttributeType(op.GetTypeName());
  switch(attr_type)
  {
  case UsdDataType::kHalf:
    {
      op.Set(GfHalf(value), timeCode);
    }
    break;

  case UsdDataType::kFloat:
    {
      op.Set(float(value), timeCode);
    }
    break;

  case UsdDataType::kDouble:
    {
      op.Set(double(value), timeCode);
    }
    break;

  case UsdDataType::kInt:
    {
      op.Set(int32_t(value), timeCode);
    }
    break;

  default:
    break;
  }
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::readRotation(MEulerRotation& result, const UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::readRotation %f %f %f\n%s\n", result.x, result.y, result.z, op.GetOpName().GetText());
  const double degToRad = 3.141592654 / 180.0;
  switch(op.GetOpType())
  {
  case UsdGeomXformOp::TypeRotateX:
    {
      result.x = readDouble(op, timeCode) * degToRad;
      result.y = 0.0;
      result.z = 0.0;
      result.order = MEulerRotation::kXYZ;
    }
    break;

  case UsdGeomXformOp::TypeRotateY:
    {
      result.x = 0.0;
      result.y = readDouble(op, timeCode) * degToRad;
      result.z = 0.0;
      result.order = MEulerRotation::kXYZ;
    }
    break;

  case UsdGeomXformOp::TypeRotateZ:
    {
      result.x = 0.0;
      result.y = 0.0;
      result.z = readDouble(op, timeCode) * degToRad;
      result.order = MEulerRotation::kXYZ;
    }
    break;

  case UsdGeomXformOp::TypeRotateXYZ:
    {
      MVector v;
      if(readVector(v, op, timeCode))
      {
        result.x = v.x * degToRad;
        result.y = v.y * degToRad;
        result.z = v.z * degToRad;
        result.order = MEulerRotation::kXYZ;
      }
      else
        return false;
    }
    break;

  case UsdGeomXformOp::TypeRotateXZY:
    {
      MVector v;
      if(readVector(v, op, timeCode))
      {
        result.x = v.x * degToRad;
        result.y = v.y * degToRad;
        result.z = v.z * degToRad;
        result.order = MEulerRotation::kXZY;
      }
      else
        return false;
    }
    break;

  case UsdGeomXformOp::TypeRotateYXZ:
    {
      MVector v;
      if(readVector(v, op, timeCode))
      {
        result.x = v.x * degToRad;
        result.y = v.y * degToRad;
        result.z = v.z * degToRad;
        result.order = MEulerRotation::kYXZ;
      }
      else
        return false;
    }
    break;

  case UsdGeomXformOp::TypeRotateYZX:
    {
      MVector v;
      if(readVector(v, op, timeCode))
      {
        result.x = v.x * degToRad;
        result.y = v.y * degToRad;
        result.z = v.z * degToRad;
        result.order = MEulerRotation::kYZX;
      }
      else
        return false;
    }
    break;

  case UsdGeomXformOp::TypeRotateZXY:
    {
      MVector v;
      if(readVector(v, op, timeCode))
      {
        result.x = v.x * degToRad;
        result.y = v.y * degToRad;
        result.z = v.z * degToRad;
        result.order = MEulerRotation::kZXY;
      }
      else
        return false;
    }
    break;

  case UsdGeomXformOp::TypeRotateZYX:
    {
      MVector v;
      if(readVector(v, op, timeCode))
      {
        result.x = v.x * degToRad;
        result.y = v.y * degToRad;
        result.z = v.z * degToRad;
        result.order = MEulerRotation::kZYX;
      }
      else
        return false;
    }
    break;

  default:
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool TransformationMatrix::pushRotation(const MEulerRotation& value, UsdGeomXformOp& op, UsdTimeCode timeCode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::pushRotation %f %f %f\n%s\n", value.x, value.y, value.z, op.GetOpName().GetText());
  const double radToDeg = 180.0 / 3.141592654;
  switch(op.GetOpType())
  {
  case UsdGeomXformOp::TypeRotateX:
    {
      pushDouble(value.x * radToDeg, op, timeCode);
    }
    break;

  case UsdGeomXformOp::TypeRotateY:
    {
      pushDouble(value.y * radToDeg, op, timeCode);
    }
    break;

  case UsdGeomXformOp::TypeRotateZ:
    {
      pushDouble(value.z * radToDeg, op, timeCode);
    }
    break;

  case UsdGeomXformOp::TypeRotateXYZ:
  case UsdGeomXformOp::TypeRotateXZY:
  case UsdGeomXformOp::TypeRotateYXZ:
  case UsdGeomXformOp::TypeRotateYZX:
  case UsdGeomXformOp::TypeRotateZYX:
  case UsdGeomXformOp::TypeRotateZXY:
    {
      MVector v(value.x, value.y, value.z);
      v *= radToDeg;
      return pushVector(v, op, timeCode);
    }
    break;

  default:
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------------------------------------
void TransformationMatrix::initialiseToPrim(bool readFromPrim, Transform* transformNode)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::initialiseToPrim\n");

  // if not yet initialized, do not execute this code! (It will crash!).
  if(!m_prim)
    return;

  bool resetsXformStack = false;
  m_xformops = m_xform.GetOrderedXformOps(&resetsXformStack);
  m_orderedOps.clear();

  if(!resetsXformStack)
    m_flags |= kInheritsTransform;

  MTransformationMatrix::RotationOrder MrotOrder = MTransformationMatrix::kXYZ;

  m_orderedOps = PxrUsdMayaXformStack::FirstMatchingSubstack(
      m_xformops, &MrotOrder,
      PxrUsdMayaXformStack::MayaStack(),
      PxrUsdMayaXformStack::CommonStack(),
      PxrUsdMayaXformStack::MatrixStack());

  if(!m_orderedOps.empty())
  {
    m_flags |= kFromMayaSchema;

    auto opIt = m_orderedOps.begin();
    for(std::vector<UsdGeomXformOp>::const_iterator it = m_xformops.begin(), e = m_xformops.end(); it != e; ++it, ++opIt)
    {
      const PxrUsdMayaXformOpClassification& opClass = **opIt;
      if (opClass.IsInvertedTwin()) continue;

      const UsdGeomXformOp& op = *it;
      const TfToken& opName = opClass.GetName();
      if (opName == PxrUsdMayaXformStackTokens->translate)
      {
        m_flags |= kPrimHasTranslation;
        if(op.GetNumTimeSamples() > 1)
        {
          m_flags |= kAnimatedTranslation;
        }
        if(readFromPrim) {
          internal_readVector(m_translationFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::translateX).setValue(m_translationFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::translateY).setValue(m_translationFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::translateZ).setValue(m_translationFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->pivot)
      {
        m_flags |= kPrimHasPivot;
        if(readFromPrim) {
          internal_readPoint(m_scalePivotFromUsd, op);
          m_rotatePivotFromUsd = m_scalePivotFromUsd;
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotX).setValue(m_rotatePivotFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotY).setValue(m_rotatePivotFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotZ).setValue(m_rotatePivotFromUsd.z);
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotX).setValue(m_scalePivotFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotY).setValue(m_scalePivotFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotZ).setValue(m_scalePivotFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->rotatePivotTranslate)
      {
        m_flags |= kPrimHasRotatePivotTranslate;
        if(readFromPrim)
        {
          internal_readVector(m_rotatePivotTranslationFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotTranslateX).setValue(m_rotatePivotTranslationFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotTranslateY).setValue(m_rotatePivotTranslationFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotTranslateZ).setValue(m_rotatePivotTranslationFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->rotatePivot)
      {
        m_flags |= kPrimHasRotatePivot;
        if(readFromPrim)
        {
          internal_readPoint(m_rotatePivotFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotX).setValue(m_rotatePivotFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotY).setValue(m_rotatePivotFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::rotatePivotZ).setValue(m_rotatePivotFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->rotate)
      {
        m_flags |= kPrimHasRotation;
        if(op.GetNumTimeSamples() > 1)
        {
          m_flags |= kAnimatedRotation;
        }
        if(readFromPrim)
        {
          internal_readRotation(m_rotationFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::rotateX).setValue(m_rotationFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::rotateY).setValue(m_rotationFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::rotateZ).setValue(m_rotationFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->rotateAxis)
      {
        m_flags |= kPrimHasRotateAxes;
        if(readFromPrim) {
          MVector vec;
          internal_readVector(vec, op);
          MEulerRotation eulers(vec.x, vec.y, vec.z);
          m_rotateOrientationFromUsd = eulers.asQuaternion();
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::rotateAxisX).setValue(vec.x);
            MPlug(transformNode->thisMObject(), MPxTransform::rotateAxisY).setValue(vec.y);
            MPlug(transformNode->thisMObject(), MPxTransform::rotateAxisZ).setValue(vec.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->scalePivotTranslate)
      {
        m_flags |= kPrimHasScalePivotTranslate;
        if(readFromPrim) {
          internal_readVector(m_scalePivotTranslationFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotTranslateX).setValue(m_scalePivotTranslationFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotTranslateY).setValue(m_scalePivotTranslationFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotTranslateZ).setValue(m_scalePivotTranslationFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->scalePivot)
      {
        m_flags |= kPrimHasScalePivot;
        if(readFromPrim) {
          internal_readPoint(m_scalePivotFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotX).setValue(m_scalePivotFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotY).setValue(m_scalePivotFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::scalePivotZ).setValue(m_scalePivotFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->shear)
      {
        m_flags |= kPrimHasShear;
        if(op.GetNumTimeSamples() > 1)
        {
          m_flags |= kAnimatedShear;
        }
        if(readFromPrim) {
          internal_readShear(m_shearFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::shearXY).setValue(m_shearFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::shearXZ).setValue(m_shearFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::shearYZ).setValue(m_shearFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->scale)
      {
        m_flags |= kPrimHasScale;
        if(op.GetNumTimeSamples() > 1)
        {
          m_flags |= kAnimatedScale;
        }
        if(readFromPrim) {
          internal_readVector(m_scaleFromUsd, op);
          if(transformNode)
          {
            MPlug(transformNode->thisMObject(), MPxTransform::scaleX).setValue(m_scaleFromUsd.x);
            MPlug(transformNode->thisMObject(), MPxTransform::scaleY).setValue(m_scaleFromUsd.y);
            MPlug(transformNode->thisMObject(), MPxTransform::scaleZ).setValue(m_scaleFromUsd.z);
          }
        }
      }
      else if (opName == PxrUsdMayaXformStackTokens->transform)
      {
        m_flags |= kPrimHasTransform;
        m_flags |= kFromMatrix;
        m_flags |= kPushPrimToMatrix;
        if(op.GetNumTimeSamples() > 1)
        {
          m_flags |= kAnimatedMatrix;
        }

        if(readFromPrim) {
          MMatrix m;
          internal_readMatrix(m, m_xformops[0]);
          decomposeMatrix(m);
          m_scaleFromUsd = scaleValue;
          m_rotationFromUsd = rotationValue;
          m_translationFromUsd = translationValue;
          m_shearFromUsd = shearValue;
          m_scalePivotFromUsd = scalePivotValue;
          m_scalePivotTranslationFromUsd = scalePivotTranslationValue;
          m_rotatePivotFromUsd = rotatePivotValue;
          m_rotatePivotTranslationFromUsd = rotatePivotTranslationValue;
          m_rotateOrientationFromUsd = rotateOrientationValue;
        }
      }
      else
      {
        std::cerr << "TransformationMatrix::initialiseToPrim - Invalid transform operation: " << opName.GetText() << std::endl;
      }

    }
  }

  // if some animation keys are found on the transform ops, assume we have a read only viewer of the transform data.
  if(m_flags & (
      kAnimatedScale |
      kAnimatedRotation |
      kAnimatedTranslation |
      kAnimatedMatrix |
      kAnimatedShear))
  {
    m_flags &= ~kPushToPrimEnabled;
    m_flags |= kReadAnimatedValues;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void TransformationMatrix::updateToTime(const UsdTimeCode& time)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::updateToTime %d\n", time.GetValue());
  // if not yet initialized, do not execute this code! (It will crash!).
  if(!m_prim)
  {
    return;
  }

  if(m_time != time)
  {
    m_time = time;
    if(hasAnimation())
    {
      auto opIt = m_orderedOps.begin();
      for(std::vector<UsdGeomXformOp>::const_iterator it = m_xformops.begin(), e = m_xformops.end(); it != e; ++it, ++opIt)
      {
        const UsdGeomXformOp& op = *it;
        const TfToken& opName = (**opIt).GetName();
        if (opName == PxrUsdMayaXformStackTokens->translate)
        {
          if(hasAnimatedTranslation())
          {
            internal_readVector(m_translationFromUsd, op);
            MPxTransformationMatrix::translationValue = m_translationFromUsd + m_translationTweak;
          }
        }
        else if (opName == PxrUsdMayaXformStackTokens->rotate)
        {
          if(hasAnimatedRotation())
          {
            internal_readRotation(m_rotationFromUsd, op);
            MPxTransformationMatrix::rotationValue = m_rotationFromUsd;
            MPxTransformationMatrix::rotationValue.x += m_rotationTweak.x;
            MPxTransformationMatrix::rotationValue.y += m_rotationTweak.y;
            MPxTransformationMatrix::rotationValue.z += m_rotationTweak.z;
          }
        }
        else if (opName == PxrUsdMayaXformStackTokens->scale)
        {
          if(hasAnimatedScale())
          {
            internal_readVector(m_scaleFromUsd, op);
            MPxTransformationMatrix::scaleValue = m_scaleFromUsd + m_scaleTweak;
          }
        }
        else if (opName == PxrUsdMayaXformStackTokens->shear)
        {
          if(hasAnimatedShear())
          {
            internal_readShear(m_shearFromUsd, op);
            MPxTransformationMatrix::shearValue = m_shearFromUsd + m_shearTweak;
          }
        }
        else if (opName == PxrUsdMayaXformStackTokens->transform)
        {
          if(hasAnimatedMatrix())
          {
            GfMatrix4d matrix;
            op.Get<GfMatrix4d>(&matrix, getTimeCode());
            double T[3], S[3];
            matrixToSRT(matrix, S, m_rotationFromUsd, T);
            m_scaleFromUsd.x = S[0];
            m_scaleFromUsd.y = S[1];
            m_scaleFromUsd.z = S[2];
            m_translationFromUsd.x = T[0];
            m_translationFromUsd.y = T[1];
            m_translationFromUsd.z = T[2];
            MPxTransformationMatrix::rotationValue.x = m_rotationFromUsd.x + m_rotationTweak.x;
            MPxTransformationMatrix::rotationValue.y = m_rotationFromUsd.y + m_rotationTweak.y;
            MPxTransformationMatrix::rotationValue.z = m_rotationFromUsd.z + m_rotationTweak.z;
            MPxTransformationMatrix::translationValue = m_translationFromUsd + m_translationTweak;
            MPxTransformationMatrix::scaleValue = m_scaleFromUsd + m_scaleTweak;
          }
        }
      }
    }
  }
}

MStatus TransformationMatrix::insertOp(
    UsdGeomXformOp::Type opType,
    UsdGeomXformOp::Precision precision,
    const TfToken& opName,
    Flags newFlag,
    bool insertAtBeginning)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::insertOp - %s\n", opName.GetText());

  // Find an iterator pointing to the location in m_orderedOps where the given
  // maya operator should be inserted. Note that opIndex must refer to an entry in MayaStack
  // (not CommonStack, etc)
  auto findOpInsertPos = [&](size_t opIndex)
      -> std::vector<PxrUsdMayaXformStack::OpClassPtr>::iterator {

    assert(opIndex != PxrUsdMayaXformStack::NO_INDEX);

    auto& mayaStack = PxrUsdMayaXformStack::MayaStack();
    assert(opIndex < mayaStack.GetOps().size());

    // We want to iterate through m_orderedOps, finding the first one that compares equal to something
    // in the range [mayaOpIter , mayaStack.GetOps().cend() ) - so, ie, we insert before any op
    // matching our op or any of the ones after it.
    const auto mayaStart = mayaStack.GetOps().cbegin() + opIndex;
    const auto mayaEnd = mayaStack.GetOps().cend();

    // Iterate through m_orderedOps...
    auto orderedIter = m_orderedOps.begin();
    const auto orderedEnd = m_orderedOps.end();
    bool foundMatch = false;
    for(; orderedIter < orderedEnd; ++orderedIter)
    {
      // For each item in m_orderedOps, see if it compares equal to something in [mayaStart, mayaEnd)
      for (auto mayaIter = mayaStart; mayaIter < mayaEnd; ++mayaIter)
      {
        // Note that we have to compare using the PxrUsdMayaXformOpClassification::operator ==
        // We can't just rely on pointer equality, because the items in m_orderedOps may not be
        // from MayaStack - ie, they might be CommonStack
        if (*mayaIter == **orderedIter)
        {
          foundMatch = true;
          break;
        }
      }
      if (foundMatch) break;
    }
    return orderedIter;
  };

  auto addOp = [&](
      size_t opIndex,
      bool insertAtBeginning)
      -> std::vector<PxrUsdMayaXformStack::OpClassPtr>::iterator {
    assert(opIndex != PxrUsdMayaXformStack::NO_INDEX);

    auto& mayaStack = PxrUsdMayaXformStack::MayaStack();
    const PxrUsdMayaXformOpClassification& opClass = mayaStack[opIndex];
    UsdGeomXformOp op = m_xform.AddXformOp(opType, precision, opName, opClass.IsInvertedTwin());
    if (!op)
    {
      return m_orderedOps.end();
    }

    // insert our op into the correct stack location
    auto posInOps = insertAtBeginning ?
        m_orderedOps.begin()
        : findOpInsertPos(opIndex);
    auto posInXfm = insertAtBeginning ?
        m_xformops.begin()
        : m_xformops.begin() + (posInOps - m_orderedOps.begin());
    PxrUsdMayaXformStack::OpClassPtr opClassPtr(&opClass);
    m_xformops.insert(posInXfm, op);
    m_orderedOps.insert(posInOps, opClassPtr);
    return posInOps;
  };

  const PxrUsdMayaXformStack::IndexPair& opPair = PxrUsdMayaXformStack::MayaStack().FindOpIndexPair(opName);

  // Add the second first, so that if insertAtBeginning is true, they will
  // maintain the same order
  auto secondPos = m_orderedOps.end();
  if (opPair.second != PxrUsdMayaXformStack::NO_INDEX)
  {
    secondPos = addOp(opPair.second, insertAtBeginning);
    if (secondPos == m_orderedOps.end())
    {
      return MStatus::kFailure;
    }
  }
  auto firstPos = addOp(opPair.first, insertAtBeginning);
  if (firstPos == m_orderedOps.end())
  {
    if (opPair.second != PxrUsdMayaXformStack::NO_INDEX && secondPos != m_orderedOps.end())
    {
      // Undo the insertion of the other pair if something went wrong
      m_orderedOps.erase(secondPos);
    }
    return MStatus::kFailure;
  }
  m_xform.SetXformOpOrder(m_xformops, (m_flags & kInheritsTransform) == 0);
  m_flags |= newFlag;
  return MStatus::kSuccess;
}

//----------------------------------------------------------------------------------------------------------------------
// Translation
//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertTranslateOp()
{
  return insertOp(UsdGeomXformOp::TypeTranslate, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->translate, kPrimHasTranslation,
      // insertAtBeginning, because we know translate is always first in the stack,
      // so we can save a little time
      true);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::translateTo(const MVector& vector, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::translateTo %f %f %f\n", vector.x, vector.y, vector.z);
  if(isTranslateLocked())
    return MS::kSuccess;
  MStatus status = MPxTransformationMatrix::translateTo(vector, space);
  if(status)
  {
    m_translationTweak = MPxTransformationMatrix::translationValue - m_translationFromUsd;
  }
  if(pushToPrimEnabled())
  {
    // if the prim does not contain a translation, make sure we insert a transform op for that.
    if(primHasTranslation())
    {
      // helping the branch predictor
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertTranslateOp(), "error inserting Translate op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::translateBy(const MVector& vector, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::translateBy %f %f %f\n", vector.x, vector.y, vector.z);
  if(isTranslateLocked())
    return MS::kSuccess;
  MStatus status = MPxTransformationMatrix::translateBy(vector, space);
  if(status)
  {
    m_translationTweak = MPxTransformationMatrix::translationValue - m_translationFromUsd;
  }
  if(pushToPrimEnabled())
  {
    // if the prim does not contain a translation, make sure we insert a transform op for that.
    if(primHasTranslation())
    {
      // helping the branch predictor
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertTranslateOp(), "error inserting Translate op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
// Scale
//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertScaleOp()
{
  return insertOp(UsdGeomXformOp::TypeScale, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->scale, kPrimHasScale);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::scaleTo(const MVector& scale, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::scaleTo %f %f %f\n", scale.x, scale.y, scale.z);
  if(isScaleLocked())
    return MStatus::kSuccess;
  MStatus status = MPxTransformationMatrix::scaleTo(scale, space);
  if(status)
  {
    m_scaleTweak = MPxTransformationMatrix::scaleValue - m_scaleFromUsd;
  }
  if(pushToPrimEnabled())
  {
    if(primHasScale())
    {
      // helping the branch predictor
    }
    else
    if(!pushPrimToMatrix())
    {
      // rare case: add a new scale op into the prim
      AL_MAYA_CHECK_ERROR(insertScaleOp(), "error inserting Scale op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::scaleBy(const MVector& scale, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::scaleBy %f %f %f\n", scale.x, scale.y, scale.z);
  if(isScaleLocked())
    return MStatus::kSuccess;
  MStatus status = MPxTransformationMatrix::scaleBy(scale, space);
  if(status)
  {
    m_scaleTweak = MPxTransformationMatrix::scaleValue - m_scaleFromUsd;
  }
  if(pushToPrimEnabled())
  {
    if(primHasScale())
    {
      // helping the branch predictor
    }
    else
    if(!pushPrimToMatrix())
    {
      // rare case: add a new scale op into the prim
      AL_MAYA_CHECK_ERROR(insertScaleOp(), "error inserting Scale op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
// Shear
//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertShearOp()
{
  return insertOp(UsdGeomXformOp::TypeTransform, UsdGeomXformOp::PrecisionDouble,
      PxrUsdMayaXformStackTokens->shear, kPrimHasShear);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::shearTo(const MVector& shear, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::shearTo %f %f %f\n", shear.x, shear.y, shear.z);
  MStatus status = MPxTransformationMatrix::shearTo(shear, space);
  if(status)
  {
    m_scaleTweak = MPxTransformationMatrix::shearValue - m_shearFromUsd;
  }
  if(pushToPrimEnabled())
  {
    if(primHasShear())
    {
      // helping the branch predictor
    }
    else
    if(!pushPrimToMatrix())
    {
      // rare case: add a new scale op into the prim
      AL_MAYA_CHECK_ERROR(insertShearOp(), "error inserting Shear op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::shearBy(const MVector& shear, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::shearBy %f %f %f\n", shear.x, shear.y, shear.z);
  MStatus status = MPxTransformationMatrix::shearBy(shear, space);
  if(status)
  {
    m_scaleTweak = MPxTransformationMatrix::shearValue - m_shearFromUsd;
  }
  if(pushToPrimEnabled())
  {
    if(primHasShear())
    {
      // helping the branch predictor
    }
    else
    if(!pushPrimToMatrix())
    {
      // rare case: add a new scale op into the prim
      AL_MAYA_CHECK_ERROR(insertShearOp(), "error inserting Shear op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertScalePivotOp()
{
  return insertOp(UsdGeomXformOp::TypeTranslate, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->scalePivot, kPrimHasScalePivot);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::setScalePivot(const MPoint& sp, MSpace::Space space, bool balance)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setScalePivot %f %f %f\n", sp.x, sp.y, sp.z);
  MStatus status = MPxTransformationMatrix::setScalePivot(sp, space, balance);
  if(status)
  {
    m_scalePivotTweak = MPxTransformationMatrix::scalePivotValue - m_scalePivotFromUsd;
  }
  if(pushToPrimEnabled())
  {
    // Do not insert a scale pivot op if the input prim has a generic pivot.
    if(primHasScalePivot() || primHasPivot())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertScalePivotOp(), "error inserting ScalePivot op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertScalePivotTranslationOp()
{
  return insertOp(UsdGeomXformOp::TypeTranslate, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->scalePivotTranslate, kPrimHasScalePivotTranslate);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::setScalePivotTranslation(const MVector& sp, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setScalePivotTranslation %f %f %f\n", sp.x, sp.y, sp.z);
  MStatus status = MPxTransformationMatrix::setScalePivotTranslation(sp, space);
  if(status)
  {
    m_scalePivotTranslationTweak = MPxTransformationMatrix::scalePivotTranslationValue - m_scalePivotTranslationFromUsd;
  }
  if(pushToPrimEnabled())
  {
    if(primHasScalePivotTranslate())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertScalePivotTranslationOp(), "error inserting ScalePivotTranslation op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertRotatePivotOp()
{
  return insertOp(UsdGeomXformOp::TypeTranslate, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->rotatePivot, kPrimHasRotatePivot);
}


//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::setRotatePivot(const MPoint& pivot, MSpace::Space space, bool balance)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setRotatePivot %f %f %f\n", pivot.x, pivot.y, pivot.z);
  MStatus status = MPxTransformationMatrix::setRotatePivot(pivot, space, balance);
  if(status)
  {
    m_rotatePivotTweak = MPxTransformationMatrix::rotatePivotValue - m_rotatePivotFromUsd;
  }
  if(pushToPrimEnabled())
  {
    // Do not insert a rotate pivot op if the input prim has a generic pivot.
    if(primHasRotatePivot() || primHasPivot())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotatePivotOp(), "error inserting RotatePivot op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertRotatePivotTranslationOp()
{
  return insertOp(UsdGeomXformOp::TypeTranslate, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->rotatePivotTranslate, kPrimHasRotatePivotTranslate);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::setRotatePivotTranslation(const MVector &vector, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setRotatePivotTranslation %f %f %f\n", vector.x, vector.y, vector.z);
  MStatus status = MPxTransformationMatrix::setRotatePivotTranslation(vector, space);
  if(status)
  {
    m_rotatePivotTranslationTweak = MPxTransformationMatrix::rotatePivotTranslationValue - m_rotatePivotTranslationFromUsd;
  }
  if(pushToPrimEnabled())
  {
    if(primHasRotatePivotTranslate())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotatePivotTranslationOp(), "error inserting RotatePivotTranslation op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertRotateOp()
{
  UsdGeomXformOp::Type opType;
  switch(rotationOrder())
  {
  case MTransformationMatrix::kXYZ:
    opType = UsdGeomXformOp::TypeRotateXYZ;
    break;

  case MTransformationMatrix::kXZY:
    opType = UsdGeomXformOp::TypeRotateXZY;
    break;

  case MTransformationMatrix::kYXZ:
    opType = UsdGeomXformOp::TypeRotateYXZ;
    break;

  case MTransformationMatrix::kYZX:
    opType = UsdGeomXformOp::TypeRotateYZX;
    break;

  case MTransformationMatrix::kZXY:
    opType = UsdGeomXformOp::TypeRotateZXY;
    break;

  case MTransformationMatrix::kZYX:
    opType = UsdGeomXformOp::TypeRotateZYX;
    break;

  default:
    break;
  }

  return insertOp(opType, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->rotate, kPrimHasRotation);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::rotateTo(const MQuaternion &q, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::rotateTo %f %f %f %f\n", q.x, q.y, q.z, q.w);
  if(isRotateLocked())
    return MS::kSuccess;
  MStatus status = MPxTransformationMatrix::rotateTo(q, space);
  if(status)
  {
    m_rotationTweak.x = MPxTransformationMatrix::rotationValue.x - m_rotationFromUsd.x;
    m_rotationTweak.y = MPxTransformationMatrix::rotationValue.y - m_rotationFromUsd.y;
    m_rotationTweak.z = MPxTransformationMatrix::rotationValue.z - m_rotationFromUsd.z;
  }
  if(pushToPrimEnabled())
  {
    if(primHasRotation())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotateOp(), "error inserting Rotate op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::rotateBy(const MQuaternion &q, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::rotateBy %f %f %f %f\n", q.x, q.y, q.z, q.w);
  if(isRotateLocked())
    return MS::kSuccess;
  MStatus status = MPxTransformationMatrix::rotateBy(q, space);
  if(status)
  {
    m_rotationTweak.x = MPxTransformationMatrix::rotationValue.x - m_rotationFromUsd.x;
    m_rotationTweak.y = MPxTransformationMatrix::rotationValue.y - m_rotationFromUsd.y;
    m_rotationTweak.z = MPxTransformationMatrix::rotationValue.z - m_rotationFromUsd.z;
  }
  if(pushToPrimEnabled())
  {
    if(primHasRotation())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotateOp(), "error inserting Rotate op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::rotateTo(const MEulerRotation &e, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::rotateTo %f %f %f\n", e.x, e.y, e.z);
  if(isRotateLocked())
    return MS::kSuccess;
  MStatus status = MPxTransformationMatrix::rotateTo(e, space);
  if(status)
  {
    m_rotationTweak.x = MPxTransformationMatrix::rotationValue.x - m_rotationFromUsd.x;
    m_rotationTweak.y = MPxTransformationMatrix::rotationValue.y - m_rotationFromUsd.y;
    m_rotationTweak.z = MPxTransformationMatrix::rotationValue.z - m_rotationFromUsd.z;
  }
  if(pushToPrimEnabled())
  {
    if(primHasRotation())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotateOp(), "error inserting Rotate op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::rotateBy(const MEulerRotation &e, MSpace::Space space)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::rotateBy %f %f %f\n", e.x, e.y, e.z);
  if(isRotateLocked())
    return MS::kSuccess;
  MStatus status = MPxTransformationMatrix::rotateBy(e, space);
  if(status)
  {
    m_rotationTweak.x = MPxTransformationMatrix::rotationValue.x - m_rotationFromUsd.x;
    m_rotationTweak.y = MPxTransformationMatrix::rotationValue.y - m_rotationFromUsd.y;
    m_rotationTweak.z = MPxTransformationMatrix::rotationValue.z - m_rotationFromUsd.z;
  }
  if(pushToPrimEnabled())
  {
    if(primHasRotation())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotateOp(), "error inserting Rotate op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::setRotationOrder(MTransformationMatrix::RotationOrder, bool)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setRotationOrder\n");
  // do not allow people to change the rotation order here.
  // It's too hard for my feeble brain to figure out how to remap that to the USD data.
  return MS::kFailure;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::insertRotateAxesOp()
{
  return insertOp(UsdGeomXformOp::TypeRotateXYZ, UsdGeomXformOp::PrecisionFloat,
      PxrUsdMayaXformStackTokens->rotateAxis, kPrimHasRotateAxes);
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::setRotateOrientation(const MQuaternion &q, MSpace::Space space, bool balance)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setRotateOrientation %f %f %f %f\n", q.x, q.y, q.z, q.w);
  MStatus status = MPxTransformationMatrix::setRotateOrientation(q, space, balance);
  if(status)
  {
    m_rotateOrientationFromUsd = MPxTransformationMatrix::rotateOrientationValue * m_rotateOrientationTweak.inverse();
  }
  if(pushToPrimEnabled())
  {
    if(primHasRotateAxes())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotateAxesOp(), "error inserting RotateAxes op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
MStatus TransformationMatrix::setRotateOrientation(const MEulerRotation& euler, MSpace::Space space, bool balance)
{
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::setRotateOrientation %f %f %f\n", euler.x, euler.y, euler.z);
  MStatus status = MPxTransformationMatrix::setRotateOrientation(euler, space, balance);
  if(status)
  {
    m_rotateOrientationFromUsd = MPxTransformationMatrix::rotateOrientationValue * m_rotateOrientationTweak.inverse();
  }
  if(pushToPrimEnabled())
  {
    if(primHasRotateAxes())
    {
    }
    else
    if(!pushPrimToMatrix())
    {
      AL_MAYA_CHECK_ERROR(insertRotateAxesOp(), "error inserting RotateAxes op");
    }
    pushToPrim();
  }
  return status;
}

//----------------------------------------------------------------------------------------------------------------------
void TransformationMatrix::pushToPrim()
{
  // if not yet intiaialised, do not execute this code! (It will crash!).
  if(!m_prim)
    return;
  TF_DEBUG(ALUSDMAYA_EVALUATION).Msg("TransformationMatrix::pushToPrim\n");

  auto opIt = m_orderedOps.begin();
  for(std::vector<UsdGeomXformOp>::iterator it = m_xformops.begin(), e = m_xformops.end(); it != e; ++it, ++opIt)
  {
    const PxrUsdMayaXformOpClassification& opClass = **opIt;
    if (opClass.IsInvertedTwin()) continue;

    UsdGeomXformOp& op = *it;
    const TfToken& opName = opClass.GetName();
    if (opName == PxrUsdMayaXformStackTokens->translate)
    {
      internal_pushVector(MPxTransformationMatrix::translationValue, op);
      m_translationFromUsd = MPxTransformationMatrix::translationValue;
      m_translationTweak = MVector(0, 0, 0);
    }
    else if (opName == PxrUsdMayaXformStackTokens->pivot)
    {
      // is this a bug?
      internal_pushPoint(MPxTransformationMatrix::rotatePivotValue, op);
      m_rotatePivotFromUsd = MPxTransformationMatrix::rotatePivotValue;
      m_rotatePivotTweak = MPoint(0, 0, 0);
      m_scalePivotFromUsd = MPxTransformationMatrix::scalePivotValue;
      m_scalePivotTweak = MVector(0, 0, 0);
    }
    else if (opName == PxrUsdMayaXformStackTokens->rotatePivotTranslate)
    {
      internal_pushPoint(MPxTransformationMatrix::rotatePivotTranslationValue, op);
      m_rotatePivotTranslationFromUsd = MPxTransformationMatrix::rotatePivotTranslationValue;
      m_rotatePivotTranslationTweak = MVector(0, 0, 0);
    }
    else if (opName == PxrUsdMayaXformStackTokens->rotatePivot)
    {
      internal_pushPoint(MPxTransformationMatrix::rotatePivotValue, op);
      m_rotatePivotFromUsd = MPxTransformationMatrix::rotatePivotValue;
      m_rotatePivotTweak = MPoint(0, 0, 0);
    }
    else if (opName == PxrUsdMayaXformStackTokens->rotate)
    {
      internal_pushRotation(MPxTransformationMatrix::rotationValue, op);
      m_rotationFromUsd = MPxTransformationMatrix::rotationValue;
      m_rotationTweak = MEulerRotation(0, 0, 0);
    }
    else if (opName == PxrUsdMayaXformStackTokens->rotateAxis)
    {
      const double radToDeg = 180.0 / 3.141592654;
      MEulerRotation e = m_rotateOrientationFromUsd.asEulerRotation();
      MVector vec(e.x * radToDeg, e.y * radToDeg, e.z * radToDeg);
      internal_pushVector(vec, op);
    }
    else if (opName == PxrUsdMayaXformStackTokens->scalePivotTranslate)
    {
      internal_pushVector(MPxTransformationMatrix::scalePivotTranslationValue, op);
      m_scalePivotTranslationFromUsd = MPxTransformationMatrix::scalePivotTranslationValue;
      m_scalePivotTranslationTweak = MVector(0, 0, 0);
    }

    else if (opName == PxrUsdMayaXformStackTokens->scalePivot)
    {
      internal_pushPoint(MPxTransformationMatrix::scalePivotValue, op);
      m_scalePivotFromUsd = MPxTransformationMatrix::scalePivotValue;
      m_scalePivotTweak = MPoint(0, 0, 0);
    }
    else if (opName == PxrUsdMayaXformStackTokens->shear)
    {
      internal_pushShear(MPxTransformationMatrix::shearValue, op);
      m_shearFromUsd = MPxTransformationMatrix::shearValue;
      m_shearTweak = MVector(0, 0, 0);
    }

    else if (opName == PxrUsdMayaXformStackTokens->scale)
    {
      internal_pushVector(MPxTransformationMatrix::scaleValue, op);
      m_scaleFromUsd = MPxTransformationMatrix::scaleValue;
      m_scaleTweak = MVector(0, 0, 0);
    }
    else if (opName == PxrUsdMayaXformStackTokens->transform)
    {
      if(pushPrimToMatrix())
      {
        MMatrix m = MPxTransformationMatrix::asMatrix();
        op.Set(*(const GfMatrix4d*)&m, getTimeCode());
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
MMatrix TransformationMatrix::asMatrix() const
{
  // Get the current transform matrix
  MMatrix m = MPxTransformationMatrix::asMatrix();

  const double x = m_localTranslateOffset.x;
  const double y = m_localTranslateOffset.y;
  const double z = m_localTranslateOffset.z;

  m[3][0] += m[0][0] * x;
  m[3][1] += m[0][1] * x;
  m[3][2] += m[0][2] * x;
  m[3][0] += m[1][0] * y;
  m[3][1] += m[1][1] * y;
  m[3][2] += m[1][2] * y;
  m[3][0] += m[2][0] * z;
  m[3][1] += m[2][1] * z;
  m[3][2] += m[2][2] * z;

  // Let Maya know what the matrix should be
  return m;
}

//----------------------------------------------------------------------------------------------------------------------
MMatrix TransformationMatrix::asMatrix(double percent) const
{
  MMatrix m = MPxTransformationMatrix::asMatrix(percent);

  const double x = m_localTranslateOffset.x * percent;
  const double y = m_localTranslateOffset.y * percent;
  const double z = m_localTranslateOffset.z * percent;

  m[3][0] += m[0][0] * x;
  m[3][1] += m[0][1] * x;
  m[3][2] += m[0][2] * x;
  m[3][0] += m[1][0] * y;
  m[3][1] += m[1][1] * y;
  m[3][2] += m[1][2] * y;
  m[3][0] += m[2][0] * z;
  m[3][1] += m[2][1] * z;
  m[3][2] += m[2][2] * z;

  return m;
}

//----------------------------------------------------------------------------------------------------------------------
void TransformationMatrix::enableReadAnimatedValues(bool enabled)
{
  if(enabled) m_flags |= kReadAnimatedValues;
  else m_flags &= ~kReadAnimatedValues;

  // if not yet intiaialised, do not execute this code! (It will crash!).
  if(!m_prim)
    return;

  // if we are enabling push to prim, we need to see if anything has changed on the transform since the last time
  // the values were synced. I'm assuming that if a given transform attribute is not the same as the default, or
  // the prim already has a transform op for that attribute, then just call a method to make a minor adjustment
  // of nothing. This will call my code that will magically construct the transform ops in the right order.
  if(enabled)
  {
    const MVector nullVec(0, 0, 0);
    const MVector oneVec(1.0, 1.0, 1.0);
    const MPoint nullPoint(0, 0, 0);
    const MQuaternion nullQuat(0, 0, 0, 1.0);

    if(!pushPrimToMatrix())
    {
      if(primHasTranslation() || translation() != nullVec)
        translateBy(nullVec);

      if(primHasScale() || scale() != oneVec)
        scaleBy(oneVec);

      if(primHasShear() || shear() != nullVec)
        shearBy(nullVec);

      if(primHasScalePivot() || scalePivot() != nullPoint)
        setScalePivot(scalePivot(), MSpace::kTransform, false);

      if(primHasScalePivot() || scalePivot() != nullPoint)
        setScalePivotTranslation(scalePivotTranslation(), MSpace::kTransform);

      if(primHasRotatePivot() || rotatePivot() != nullPoint)
        setRotatePivot(rotatePivot(), MSpace::kTransform, false);

      if(primHasRotatePivotTranslate() || rotatePivotTranslation() != nullVec)
        setRotatePivotTranslation(rotatePivotTranslation(), MSpace::kTransform);

      if(primHasRotation() || rotation() != nullQuat)
        rotateBy(nullQuat);

      if(primHasRotateAxes() || rotateOrientation() != nullQuat)
        setRotateOrientation(rotateOrientation(), MSpace::kTransform, false);
    }
    else
    if(primHasTransform())
    {
      for(size_t i = 0, n = m_orderedOps.size(); i < n; ++i)
      {
        if(m_orderedOps[i]->GetName() == PxrUsdMayaXformStackTokens->transform)
        {
          MMatrix m = MPxTransformationMatrix::asMatrix();
          m_xformops[i].Set(*(const GfMatrix4d*)&m, getTimeCode());
          break;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
void TransformationMatrix::enablePushToPrim(bool enabled)
{
  if(enabled) m_flags |= kPushToPrimEnabled;
  else m_flags &= ~kPushToPrimEnabled;

  // if not yet intiaialised, do not execute this code! (It will crash!).
  if(!m_prim)
    return;

  // if we are enabling push to prim, we need to see if anything has changed on the transform since the last time
  // the values were synced. I'm assuming that if a given transform attribute is not the same as the default, or
  // the prim already has a transform op for that attribute, then just call a method to make a minor adjustment
  // of nothing. This will call my code that will magically construct the transform ops in the right order.
  if(enabled && getTimeCode() == UsdTimeCode::Default())
  {
    const MVector nullVec(0, 0, 0);
    const MVector oneVec(1.0, 1.0, 1.0);
    const MPoint nullPoint(0, 0, 0);
    const MQuaternion nullQuat(0, 0, 0, 1.0);

    if(!pushPrimToMatrix())
    {
      if(primHasTranslation() || translation() != nullVec)
        translateBy(nullVec);

      if(primHasScale() || scale() != oneVec)
        scaleBy(oneVec);

      if(primHasShear() || shear() != nullVec)
        shearBy(nullVec);

      if(primHasScalePivot() || scalePivot() != nullPoint)
        setScalePivot(scalePivot(), MSpace::kTransform, false);

      if(primHasScalePivotTranslate() || scalePivotTranslation() != nullPoint)
        setScalePivotTranslation(scalePivotTranslation(), MSpace::kTransform);

      if(primHasRotatePivot() || rotatePivot() != nullPoint)
        setRotatePivot(rotatePivot(), MSpace::kTransform, false);

      if(primHasRotatePivotTranslate() || rotatePivotTranslation() != nullVec)
        setRotatePivotTranslation(rotatePivotTranslation(), MSpace::kTransform);

      if(primHasRotation() || rotation() != nullQuat)
        rotateBy(nullQuat);

      if(primHasRotateAxes() || rotateOrientation() != nullQuat)
        setRotateOrientation(rotateOrientation(), MSpace::kTransform, false);
    }
    else
    if(primHasTransform())
    {
      for(size_t i = 0, n = m_orderedOps.size(); i < n; ++i)
      {
        if(m_orderedOps[i]->GetName() == PxrUsdMayaXformStackTokens->transform)
        {
          MMatrix m = MPxTransformationMatrix::asMatrix();
          m_xformops[i].Set(*(const GfMatrix4d*)&m, getTimeCode());
          break;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
} // nodes
} // usdmaya
} // AL
//----------------------------------------------------------------------------------------------------------------------
