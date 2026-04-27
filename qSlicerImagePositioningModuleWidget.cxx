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

// Qt includes
#include <QDebug>
#include <QSignalBlocker>

// Slicer includes
#include "qSlicerImagePositioningModuleWidget.h"
#include "ui_qSlicerImagePositioningModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkImageData.h>


// Slicer includes
//#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>

#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

// Logic includes
#include <vtkSlicerImagePositioningLogic.h>

class QAbstractButton;

//-----------------------------------------------------------------------------
class qSlicerImagePositioningModuleWidgetPrivate: public Ui_qSlicerImagePositioningModuleWidget
{
public:
  qSlicerImagePositioningModuleWidgetPrivate();
  enum BeamOrientation
  {
    BeamOrientationNone = 0,
    BeamOrientationHorizontal,
    BeamOrientationVertical
  };
  struct Pose2D
  {
    double HorizontalOffsetMm = 0.0;
    double VerticalOffsetMm = 0.0;
    double RotationDeg = 0.0;
  };

  const char* GENERIC_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\" split=\"true\" >"\
    "  <item splitSize=\"500\"> "\
    "    <layout type=\"horizontal\">"\
    "    <item>" \
    "     <view class=\"vtkMRMLViewNode\" singletontag=\"1\">" \
    "       <property name=\"viewlabel\" action=\"default\">1</property>" \
    "     </view>" \
    "    </item>" \
    "     <item>" \
    "       <view class=\"vtkMRMLSliceNode\" singletontag=\"XrayDetectorSlice\">" \
    "         <property name=\"orientation\" action=\"default\">Axial</property>" \
    "         <property name=\"viewlabel\" action=\"default\">C</property>" \
    "         <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "         <property name=\"viewgroup\" action=\"default\">101</property>" \
    "       </view>" \
    "     </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"350\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">" \
    "           <property name=\"orientation\" action=\"default\">Axial</property>" \
    "           <property name=\"viewlabel\" action=\"default\">R</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#F34A33</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">" \
    "           <property name=\"orientation\" action=\"default\">Coronal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">G</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#6EB04B</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">" \
    "           <property name=\"orientation\" action=\"default\">Sagittal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">Y</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#EDD54C</property>" \
    "        </view>" \
    "      </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"0\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "        <view class=\"vtkMRMLViewNode\" singletontag=\"XrayDetectorSlice\">" \
    "           <property name=\"viewlabel\" action=\"default\">F</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "           <property name=\"viewgroup\" action=\"default\">100</property>" \
    "        </view>" \
    "       </item>" \
    "    </layout>" \
    "  </item>" \
    "</layout>";
  const int GENERIC_LAYOUT_ID = 1020;
  int PreviousLayoutId = -1;
  BeamOrientation ActiveOrientation = BeamOrientationNone;
  Pose2D HorizontalPose;
  Pose2D VerticalPose;
  vtkMRMLLinearTransformNode* HorizontalImageTransformNode = nullptr;
  vtkMRMLLinearTransformNode* VerticalImageTransformNode = nullptr;

//  vtkSmartPointer<vtkMRMLImagePositioningNode> ImagePositioningNode;
  vtkSlicerImagePositioningLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidgetPrivate::qSlicerImagePositioningModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidget::qSlicerImagePositioningModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerImagePositioningModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidget::~qSlicerImagePositioningModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setup()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    if (!layoutNode->SetLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION))
    {
      layoutNode->AddLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION);
    }
  }
  // Nodes
  connect(d->MRMLNodeComboBox_DrrImage, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onDrrImageNodeChanged(vtkMRMLNode*)));
  connect(d->MRMLNodeComboBox_XrayImage, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onXrayImageNodeChanged(vtkMRMLNode*)));
  //connect(d->MRMLSliderWidget_DrrOpacity, SIGNAL(valueChanged(double)),
  //    this, SLOT(onDrrOpacityChanged(double)));
  connect(d->MRMLSliderWidget_XrayOpacity, SIGNAL(valueChanged(double)),
      this, SLOT(onXrayOpacityChanged(double)));


  // Buttons
  // TODO: Add proper mutual exclusivity for buttons
  QObject::connect( d->PushButton_SetView, SIGNAL(clicked()),
    this, SLOT(onSetViewClicked()));
  QObject::connect( d->PushButton_CustomLayout, SIGNAL(clicked()),
    this, SLOT(onSetCustomLayoutClicked()));
  QObject::connect(d->PushButton_HorizontalImage, SIGNAL(clicked()),
      this, SLOT(onHorizontalImageClicked()));
  QObject::connect(d->PushButton_VerticalImage, SIGNAL(clicked()),
      this, SLOT(onVerticalImageClicked()));
  QObject::connect(d->MRMLSliderWidget_HorizontalTransform, SIGNAL(valueChanged(double)),
    this, SLOT(onHorizontalTransformChanged(double)));
  QObject::connect(d->MRMLSliderWidget_VerticalTransform, SIGNAL(valueChanged(double)),
    this, SLOT(onVerticalTransformChanged(double)));
  QObject::connect(d->MRMLSliderWidget_Rotation, SIGNAL(valueChanged(double)),
    this, SLOT(onRotationTransformChanged(double)));
  QObject::connect(d->PushButton_Up, SIGNAL(clicked()), this, SLOT(onMoveUpClicked()));
  QObject::connect(d->PushButton_Down, SIGNAL(clicked()), this, SLOT(onMoveDownClicked()));
  QObject::connect(d->PushButton_Left, SIGNAL(clicked()), this, SLOT(onMoveLeftClicked()));
  QObject::connect(d->PushButton_Right, SIGNAL(clicked()), this, SLOT(onMoveRightClicked()));
  QObject::connect(d->PushButton_Clockwise, SIGNAL(clicked()), this, SLOT(onRotateClockwiseClicked()));
  QObject::connect(d->PushButton_CounterClockwise, SIGNAL(clicked()), this, SLOT(onRotateCounterClockwiseClicked()));
  QObject::connect(d->PushButton_Reset, SIGNAL(clicked()), this, SLOT(onResetTransformClicked()));

  this->setXrayNode(nullptr);
  this->sync2DControlsFromActiveOrientation();
}


void qSlicerImagePositioningModuleWidget::onSetCustomLayoutClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  if (d->PushButton_CustomLayout->isChecked())
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->GENERIC_LAYOUT_ID);
      qDebug() << Q_FUNC_INFO << layoutManager->layout();
      d->MRMLSliderWidget_XrayOpacity->setEnabled(true);
    }
  }
  else
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->PreviousLayoutId);
      qDebug() << Q_FUNC_INFO << layoutManager->layout();
      d->MRMLSliderWidget_XrayOpacity->setEnabled(false);
    }
  }
  slicerApplication->processEvents();
}

void qSlicerImagePositioningModuleWidget::onSetViewClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->MRMLNodeComboBox_XrayImage->currentNode() || !d->MRMLNodeComboBox_DrrImage->currentNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid Xray or DRR image node";
    return;
  }

  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  // Set images
  // DRR - background, XRay - foreground
  if (layoutManager->layout() == 1020)
  {
      vtkMRMLScalarVolumeNode* xrayImageNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_XrayImage->currentNode());
      vtkMRMLScalarVolumeNode* drrImageNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_DrrImage->currentNode());

      qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget("XrayDetectorSlice");
      vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
      vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();
      sliceLogic->GetSliceCompositeNode()->SetForegroundVolumeID(xrayImageNode->GetID());
      sliceLogic->GetSliceCompositeNode()->SetBackgroundVolumeID(drrImageNode->GetID());
      sliceLogic->GetSliceCompositeNode()->SetForegroundOpacity(0.5);
      sliceLogic->GetSliceCompositeNode()->SetClipToBackgroundVolume(false);
      sliceLogic->RotateSliceToLowestVolumeAxes(); // Reformat 
      sliceLogic->FitSliceToAll();
      sliceNode->UpdateMatrices();
      setSliceOrientation();

      // Change color of images
      vtkMRMLScalarVolumeDisplayNode* drrDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(drrImageNode->GetDisplayNode());
      vtkMRMLScalarVolumeDisplayNode* xrayDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(xrayImageNode->GetDisplayNode());
      drrDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGreen");
      xrayDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRed");

      // Set compositing mode
      // TODO: let user choose compositing mode
      // alpha blend = 0
      // add = 2
      // subtract = 3
      sliceLogic->GetSliceCompositeNode()->SetCompositing(2); // add
  }
  else
  {
      qCritical() << Q_FUNC_INFO << ": Wrong layout, set layout to custom";
      return;
  }
}

void qSlicerImagePositioningModuleWidget::onHorizontalImageClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (d->PushButton_VerticalImage->isChecked())
  {
    d->PushButton_VerticalImage->setChecked(false);
  }
  d->ActiveOrientation = qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationHorizontal;
//  this->setSliceOrientation();
  this->setXrayNode(vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_XrayImage->currentNode()));
  this->sync2DControlsFromActiveOrientation();
  this->updateXrayTransformFrom2DControls();

}

void qSlicerImagePositioningModuleWidget::onVerticalImageClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (d->PushButton_HorizontalImage->isChecked())
  {
    d->PushButton_HorizontalImage->setChecked(false);
  }
  d->ActiveOrientation = qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical;
//  this->setSliceOrientation();
  this->setXrayNode(vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_XrayImage->currentNode()));
  this->sync2DControlsFromActiveOrientation();
  this->updateXrayTransformFrom2DControls();
}

void qSlicerImagePositioningModuleWidget::onDrrImageNodeChanged(vtkMRMLNode* drrImageNode)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (drrImageNode)
  {
    qDebug() << Q_FUNC_INFO << drrImageNode->GetName();
  }
  else 
  {
    qCritical() << Q_FUNC_INFO << ": Invalid DRR image node";
    d->PushButton_SetView->setEnabled(false);
    return;
  }

  if (d->MRMLNodeComboBox_XrayImage->currentNode() && d->MRMLNodeComboBox_DrrImage->currentNode())
  {
    d->PushButton_SetView->setEnabled(true);
  }
}

void qSlicerImagePositioningModuleWidget::onXrayImageNodeChanged(vtkMRMLNode* xrayImageNode)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  vtkMRMLScalarVolumeNode* xrayVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(xrayImageNode);
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationNone)
  {
    d->ActiveOrientation = qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationHorizontal;
  }
  this->setXrayNode(xrayVolumeNode);
  this->sync2DControlsFromActiveOrientation();
  this->updateXrayTransformFrom2DControls();

  if (xrayImageNode)
  {
    qDebug() << Q_FUNC_INFO << xrayImageNode->GetName();
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Invalid Xray image node";
    d->PushButton_SetView->setEnabled(false);
    return;
  }
  if (d->MRMLNodeComboBox_XrayImage->currentNode() && d->MRMLNodeComboBox_DrrImage->currentNode())
  {
    d->PushButton_SetView->setEnabled(true);
  }
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setXrayNode(vtkMRMLScalarVolumeNode* xrayNode)
{
  Q_D(qSlicerImagePositioningModuleWidget);

  vtkMRMLLinearTransformNode* activeTransformNode = nullptr;
  if (xrayNode)
  {
    vtkMRMLScene* scene = xrayNode->GetScene();
    if (!scene)
    {
      qCritical() << Q_FUNC_INFO << ": Xray node has no scene";
    }
    else
    {
      d->HorizontalImageTransformNode = this->getOrCreateNamedTransformNode(scene, "XrayTransformHorizontal");
      d->VerticalImageTransformNode = this->getOrCreateNamedTransformNode(scene, "XrayTransformVertical");
      activeTransformNode = this->getActiveXrayTransformNode();
      if (!activeTransformNode)
      {
        activeTransformNode = d->HorizontalImageTransformNode;
      }
      if (activeTransformNode)
      {
        xrayNode->SetAndObserveTransformNodeID(activeTransformNode->GetID());
      }
    }
  }

  d->MRMLMatrixWidget_TransformMatrix->setMRMLTransformNode(activeTransformNode);
  const bool hasTransform = (activeTransformNode != nullptr);
  d->MRMLMatrixWidget_TransformMatrix->setEnabled(hasTransform);
  d->MRMLSliderWidget_HorizontalTransform->setEnabled(hasTransform);
  d->MRMLSliderWidget_VerticalTransform->setEnabled(hasTransform);
  d->MRMLSliderWidget_Rotation->setEnabled(hasTransform);
  d->PushButton_Up->setEnabled(hasTransform);
  d->PushButton_Down->setEnabled(hasTransform);
  d->PushButton_Left->setEnabled(hasTransform);
  d->PushButton_Right->setEnabled(hasTransform);
  d->PushButton_Clockwise->setEnabled(hasTransform);
  d->PushButton_CounterClockwise->setEnabled(hasTransform);
  d->PushButton_Reset->setEnabled(hasTransform);
  d->MRMLCoordinatesWidget_TranslatePosition->setEnabled(hasTransform);
}

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* qSlicerImagePositioningModuleWidget::getOrCreateNamedTransformNode(
  vtkMRMLScene* scene, const char* name)
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

//-----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* qSlicerImagePositioningModuleWidget::getActiveXrayTransformNode()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical)
  {
    return d->VerticalImageTransformNode;
  }
  return d->HorizontalImageTransformNode;
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::sync2DControlsFromActiveOrientation()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  qSlicerImagePositioningModuleWidgetPrivate::Pose2D* pose = &d->HorizontalPose;
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical)
  {
    pose = &d->VerticalPose;
  }
  QSignalBlocker horizontalBlocker(d->MRMLSliderWidget_HorizontalTransform);
  QSignalBlocker verticalBlocker(d->MRMLSliderWidget_VerticalTransform);
  QSignalBlocker rotationBlocker(d->MRMLSliderWidget_Rotation);
  d->MRMLSliderWidget_HorizontalTransform->setValue(pose->HorizontalOffsetMm);
  d->MRMLSliderWidget_VerticalTransform->setValue(pose->VerticalOffsetMm);
  d->MRMLSliderWidget_Rotation->setValue(pose->RotationDeg);
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::updateXrayTransformFrom2DControls()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  vtkMRMLScalarVolumeNode* xrayNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBox_XrayImage->currentNode());
  vtkMRMLLinearTransformNode* activeTransformNode = this->getActiveXrayTransformNode();
  if (!xrayNode || !activeTransformNode)
  {
    return;
  }

  vtkImageData* imageData = xrayNode->GetImageData();
  if (!imageData)
  {
    qCritical() << Q_FUNC_INFO << ": Xray node has no image data";
    return;
  }

  qSlicerImagePositioningModuleWidgetPrivate::Pose2D* pose = &d->HorizontalPose;
  bool isHorizontal = true;
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical)
  {
    pose = &d->VerticalPose;
    isHorizontal = false;
  }

  double eHorizontal[3] = { 0.0, 0.0, 1.0 };
  double eVertical[3] = { 0.0, 1.0, 0.0 };
  double normal[3] = { 1.0, 0.0, 0.0 };
  if (!isHorizontal)
  {
    eHorizontal[0] = 1.0; eHorizontal[1] = 0.0; eHorizontal[2] = 0.0; // L-R
    eVertical[0] = 0.0; eVertical[1] = 0.0; eVertical[2] = 1.0;       // S-I
    normal[0] = 0.0; normal[1] = -1.0; normal[2] = 0.0;               // A-P
  }

  double translationRAS[3] =
  {
    pose->HorizontalOffsetMm * eHorizontal[0] + pose->VerticalOffsetMm * eVertical[0],
    pose->HorizontalOffsetMm * eHorizontal[1] + pose->VerticalOffsetMm * eVertical[1],
    pose->HorizontalOffsetMm * eHorizontal[2] + pose->VerticalOffsetMm * eVertical[2]
  };
  // ^ Simplify this crap

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
 // xrayTransform->PostMultiply();
  xrayTransform->Identity();
  xrayTransform->Translate(centerRAS4[0], centerRAS4[1], centerRAS4[2]);
  xrayTransform->Translate(translationRAS[0], translationRAS[1], translationRAS[2]);
  xrayTransform->RotateWXYZ(pose->RotationDeg, normal);
  xrayTransform->Translate(-centerRAS4[0], -centerRAS4[1], -centerRAS4[2]);

  activeTransformNode->SetMatrixTransformToParent(xrayTransform->GetMatrix());
  d->MRMLCoordinatesWidget_TranslatePosition->setCoordinates(translationRAS);
  d->MRMLMatrixWidget_TransformMatrix->setMRMLTransformNode(activeTransformNode);
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveUpClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_VerticalTransform->setValue(
    d->MRMLSliderWidget_VerticalTransform->value() + d->MRMLSliderWidget_VerticalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveDownClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_VerticalTransform->setValue(
    d->MRMLSliderWidget_VerticalTransform->value() - d->MRMLSliderWidget_VerticalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveLeftClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_HorizontalTransform->setValue(
    d->MRMLSliderWidget_HorizontalTransform->value() - d->MRMLSliderWidget_HorizontalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveRightClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_HorizontalTransform->setValue(
    d->MRMLSliderWidget_HorizontalTransform->value() + d->MRMLSliderWidget_HorizontalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onRotateClockwiseClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_Rotation->setValue(
    d->MRMLSliderWidget_Rotation->value() + d->MRMLSliderWidget_Rotation->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onRotateCounterClockwiseClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_Rotation->setValue(
    d->MRMLSliderWidget_Rotation->value() - d->MRMLSliderWidget_Rotation->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onResetTransformClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical)
  {
    d->VerticalPose = qSlicerImagePositioningModuleWidgetPrivate::Pose2D();
  }
  else
  {
    d->HorizontalPose = qSlicerImagePositioningModuleWidgetPrivate::Pose2D();
  }
  this->sync2DControlsFromActiveOrientation();
  this->updateXrayTransformFrom2DControls();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onHorizontalTransformChanged(double value)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical)
  {
    d->VerticalPose.HorizontalOffsetMm = value;
  }
  else
  {
    d->HorizontalPose.HorizontalOffsetMm = value;
  }
  this->updateXrayTransformFrom2DControls();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onVerticalTransformChanged(double value)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical)
  {
    d->VerticalPose.VerticalOffsetMm = value;
  }
  else
  {
    d->HorizontalPose.VerticalOffsetMm = value;
  }
  this->updateXrayTransformFrom2DControls();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onRotationTransformChanged(double value)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (d->ActiveOrientation == qSlicerImagePositioningModuleWidgetPrivate::BeamOrientationVertical)
  {
    d->VerticalPose.RotationDeg = value;
  }
  else
  {
    d->HorizontalPose.RotationDeg = value;
  }
  this->updateXrayTransformFrom2DControls();
}

//void qSlicerImagePositioningModuleWidget::onDrrOpacityChanged(double* opacity)
//{
//    Q_D(qSlicerImagePositioningModuleWidget);
//    qSlicerApplication* slicerApplication = qSlicerApplication::application();
//    qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
//
//    if (layoutManager->layout() == 1020)
//    {
//        qMRMLSliceWidget* sliceWidget = slicerApplication->layoutManager()->sliceWidget("XrayDetectorSlice");
//        vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();
//        sliceLogic->GetSliceCompositeNode()->SetBackgroundOpacity(opacity);
//    }
//    else
//    {
//        qCritical() << Q_FUNC_INFO << ": Wrong layout, set layout to custom";
//        return;
//    }
//}

void qSlicerImagePositioningModuleWidget::onXrayOpacityChanged(double opacity)
{
    Q_D(qSlicerImagePositioningModuleWidget);
    qSlicerApplication* slicerApplication = qSlicerApplication::application();
    qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

    if (layoutManager->layout() == 1020)
    {
        qMRMLSliceWidget* sliceWidget = slicerApplication->layoutManager()->sliceWidget("XrayDetectorSlice");
        vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();
        sliceLogic->GetSliceCompositeNode()->SetForegroundOpacity(opacity);
    }
    else
    {
        qCritical() << Q_FUNC_INFO << ": Wrong layout, set layout to custom";
        return;
    }

}

void qSlicerImagePositioningModuleWidget::setSliceOrientation()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget("XrayDetectorSlice");
  vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();

  double N[3];
  double T[3];

  if (d->PushButton_HorizontalImage->isChecked() && !d->PushButton_VerticalImage->isChecked())
  {
    // Horizontal
    N[0] = -1.0; N[1] = 0.0; N[2] = 0.0; // Normal: Left -> Right
    T[0] = 0.0; T[1] = 0.0; T[2] = 1.0; // Up: Anterior
  }
  else if (d->PushButton_VerticalImage->isChecked() && !d->PushButton_HorizontalImage->isChecked())
  {
    // Vertical
    N[0] = 0.0; N[1] = -1.0; N[2] = 0.0; // Normal: Anterior -> Posterior
    T[0] = 1.0; T[1] = 0.0; T[2] = 0.0; // Up: Superior
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Set a proper beam orientation";
    return;
  }

  vtkMatrix4x4* SliceToRASMatrix = sliceNode->GetSliceToRAS();

  double originRAS[3];
  originRAS[0] = SliceToRASMatrix->GetElement(0,3);
  originRAS[1] = SliceToRASMatrix->GetElement(1,3);
  originRAS[2] = SliceToRASMatrix->GetElement(2,3);

  sliceNode->SetSliceToRASByNTP(
      N[0], N[1], N[2],
      T[0], T[1], T[2],
      originRAS[0], originRAS[1], originRAS[2],
      0                   // Orientation index (unused)
  );
  sliceNode->UpdateMatrices();
}