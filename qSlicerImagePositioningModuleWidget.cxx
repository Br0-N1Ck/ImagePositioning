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
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSliceCompositeNode.h>


// Slicer includes
//#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>

#include <vtkTransform.h>

class QAbstractButton;

//-----------------------------------------------------------------------------
class qSlicerImagePositioningModuleWidgetPrivate: public Ui_qSlicerImagePositioningModuleWidget
{
public:
  qSlicerImagePositioningModuleWidgetPrivate();

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
  vtkSmartPointer<vtkTransform> DrrImageTransform;
  vtkSmartPointer<vtkTransform> XrayImageTransform;
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
  QObject::connect( d->PushButton_SetView, SIGNAL(clicked()),
    this, SLOT(onSetViewClicked()));
  QObject::connect( d->PushButton_CustomLayout, SIGNAL(clicked()),
    this, SLOT(onSetCustomLayoutClicked()));
  QObject::connect(d->PushButton_HorizontalImage, SIGNAL(clicked()),
      this, SLOT(onHorizontalImageClicked()));
  QObject::connect(d->PushButton_VerticalImage, SIGNAL(clicked()),
      this, SLOT(onVerticalImageClicked()));
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
      vtkMRMLNode* xrayImageNode = d->MRMLNodeComboBox_XrayImage->currentNode();
      vtkMRMLNode* drrImageNode = d->MRMLNodeComboBox_DrrImage->currentNode();

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

    // Uncheck "Vertical" button
    if (d->PushButton_VerticalImage->isChecked())
    {
        d->PushButton_VerticalImage->setChecked(false);
    }

}

void qSlicerImagePositioningModuleWidget::onVerticalImageClicked()
{
    Q_D(qSlicerImagePositioningModuleWidget);

    // Uncheck "Horizontal" button
    if (d->PushButton_HorizontalImage->isChecked())
    {
        d->PushButton_HorizontalImage->setChecked(false);
    }
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
    N[0] = 1.0; N[1] = 0.0; N[2] = 0.0; // Normal: Left -> Right
    T[0] = 0.0; T[1] = 0.0; T[2] = -1.0; // Up: Anterior
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