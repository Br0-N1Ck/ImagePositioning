/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// ImagePositioning Logic includes
#include "vtkSlicerImagePositioningLogic.h"
#include "vtkMRMLImagePositioningNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkImageData.h>

// STD includes
#include <cassert>
#include <string>

namespace
{
vtkMRMLLinearTransformNode* GetOrCreateNamedTransformNode(vtkMRMLScene* scene, const char* name)
{
  if (!scene || !name)
  {
    return nullptr;
  }

  vtkMRMLLinearTransformNode* linearTransformNode =
    vtkMRMLLinearTransformNode::SafeDownCast(scene->GetFirstNodeByName(name));
  if (linearTransformNode)
  {
    return linearTransformNode;
  }

  vtkNew<vtkMRMLLinearTransformNode> newTransformNode;
  newTransformNode->SetName(name);
  scene->AddNode(newTransformNode);
  return newTransformNode.GetPointer();
}
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerImagePositioningLogic);

//----------------------------------------------------------------------------
vtkSlicerImagePositioningLogic::vtkSlicerImagePositioningLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerImagePositioningLogic::~vtkSlicerImagePositioningLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic::RegisterNodes()
{
    vtkMRMLScene* scene = this->GetMRMLScene();
    if (!scene)
    {
        return;
    }

    if (!scene->IsNodeClassRegistered("vtkMRMLImagePositioningNode"))
    {
        scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLImagePositioningNode>::New());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
    if (!node || !this->GetMRMLScene())
    {
        vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
        return;
    }

    if (node->IsA("vtkMRMLImagePositioningNode"))
    {
        vtkNew<vtkIntArray> events;
        events->InsertNextValue(vtkCommand::ModifiedEvent);
        vtkObserveMRMLNodeEventsMacro(node, events);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
vtkMRMLImagePositioningNode* vtkSlicerImagePositioningLogic::GetOrCreateImagePositioningNode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return nullptr;
  }

  vtkMRMLImagePositioningNode* imagePositioningNode = vtkMRMLImagePositioningNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkMRMLImagePositioningNode"));
  if (imagePositioningNode)
  {
    return imagePositioningNode;
  }

  vtkNew<vtkMRMLImagePositioningNode> newImagePositioningNode;
  scene->AddNode(newImagePositioningNode);
  return newImagePositioningNode.GetPointer();
}

//---------------------------------------------------------------------------
//void vtkSlicerImagePositioningLogic::SetDrrNode(vtkMRMLScalarVolumeNode* drrNode)
//{
//  vtkMRMLImagePositioningNode* imagePositioningNode = this->GetOrCreateImagePositioningNode();
//  if (!imagePositioningNode)
//  {
//    return;
//  }
//  imagePositioningNode->SetAndObserveDrrNode(drrNode);
//}

//---------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic::SetXrayNode(vtkMRMLScalarVolumeNode* xrayNode)
{
  vtkMRMLImagePositioningNode* imagePositioningNode = this->GetOrCreateImagePositioningNode();
  if (!imagePositioningNode)
  {
    return;
  }
  imagePositioningNode->SetAndObserveXrayNode(xrayNode);
  if (!xrayNode)
  {
    return;
  }

  vtkMRMLScene* scene = xrayNode->GetScene();
  if (!scene)
  {
    return;
  }

  vtkMRMLLinearTransformNode* horizontalTransformNode =
    GetOrCreateNamedTransformNode(scene, "XrayTransformHorizontal");
  vtkMRMLLinearTransformNode* verticalTransformNode =
    GetOrCreateNamedTransformNode(scene, "XrayTransformVertical");
  imagePositioningNode->SetAndObserveHorizontalTransformNode(horizontalTransformNode);
  imagePositioningNode->SetAndObserveVerticalTransformNode(verticalTransformNode);

  vtkMRMLLinearTransformNode* activeTransformNode = this->GetActiveXrayTransformNode();
  if (activeTransformNode)
  {
    xrayNode->SetAndObserveTransformNodeID(activeTransformNode->GetID());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic::SetActiveOrientation(int orientation)
{
  vtkMRMLImagePositioningNode* imagePositioningNode = this->GetOrCreateImagePositioningNode();
  if (!imagePositioningNode)
  {
    return;
  }
  imagePositioningNode->SetActiveOrientation(orientation);

  vtkMRMLScalarVolumeNode* xrayNode = imagePositioningNode->GetXrayNode();
  vtkMRMLLinearTransformNode* activeTransformNode = this->GetActiveXrayTransformNode();
  if (xrayNode && activeTransformNode)
  {
    xrayNode->SetAndObserveTransformNodeID(activeTransformNode->GetID());
  }
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerImagePositioningLogic::GetActiveXrayTransformNode()
{
  vtkMRMLImagePositioningNode* imagePositioningNode = this->GetOrCreateImagePositioningNode();
  if (!imagePositioningNode)
  {
    return nullptr;
  }

  if (imagePositioningNode->GetActiveOrientation() == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    return imagePositioningNode->GetVerticalTransformNode();
  }
  return imagePositioningNode->GetHorizontalTransformNode();
}

//---------------------------------------------------------------------------
void vtkSlicerImagePositioningLogic::ApplyXray2DTransform(
  double horizontalOffsetMm, double verticalOffsetMm, double rotationDeg, int orientation)
{
  vtkMRMLImagePositioningNode* imagePositioningNode = this->GetOrCreateImagePositioningNode();
  if (!imagePositioningNode)
  {
    return;
  }

  vtkMRMLScalarVolumeNode* xrayNode = imagePositioningNode->GetXrayNode();
  vtkMRMLLinearTransformNode* activeTransformNode = this->GetActiveXrayTransformNode();
  if (!xrayNode || !activeTransformNode)
  {
    return;
  }

  vtkImageData* imageData = xrayNode->GetImageData();
  if (!imageData)
  {
    return;
  }


  double eHorizontal[3] = { 0.0, 0.0, 1.0 };
  double eVertical[3] = { 0.0, 1.0, 0.0 };
  double normal[3] = { 1.0, 0.0, 0.0 };
  if (orientation == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    eHorizontal[0] = 1.0; eHorizontal[1] = 0.0; eHorizontal[2] = 0.0; // L-R
    eVertical[0] = 0.0; eVertical[1] = 0.0; eVertical[2] = 1.0;       // S-I
    normal[0] = 0.0; normal[1] = -1.0; normal[2] = 0.0;               // A-P
  }

  const double translationRAS[3] =
  {
    horizontalOffsetMm * eHorizontal[0] + verticalOffsetMm * eVertical[0],
    horizontalOffsetMm * eHorizontal[1] + verticalOffsetMm * eVertical[1],
    horizontalOffsetMm * eHorizontal[2] + verticalOffsetMm * eVertical[2]
  };

  int dimensions[3] = { 0, 0, 0 };
  imageData->GetDimensions(dimensions);
  double centerIJK[4] =
  {
    0.5 * static_cast<double>(dimensions[0] - 1),
    0.5 * static_cast<double>(dimensions[1] - 1),
    0.5 * static_cast<double>(dimensions[2] - 1),
    1.0
  };
  vtkNew<vtkMatrix4x4> ijkToRASMatrix;
  xrayNode->GetIJKToRASMatrix(ijkToRASMatrix);
  double centerRAS4[4] = { 0.0, 0.0, 0.0, 1.0 };
  ijkToRASMatrix->MultiplyPoint(centerIJK, centerRAS4);

  vtkNew<vtkTransform> xrayTransform;
  xrayTransform->Identity();
  xrayTransform->Translate(centerRAS4[0], centerRAS4[1], centerRAS4[2]);
  xrayTransform->Translate(translationRAS[0], translationRAS[1], translationRAS[2]);
  xrayTransform->RotateWXYZ(rotationDeg, normal);
  xrayTransform->Translate(-centerRAS4[0], -centerRAS4[1], -centerRAS4[2]);

  activeTransformNode->SetMatrixTransformToParent(xrayTransform->GetMatrix());
}
