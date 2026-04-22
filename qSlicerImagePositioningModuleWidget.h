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

#ifndef __qSlicerImagePositioningModuleWidget_h
#define __qSlicerImagePositioningModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerImagePositioningModuleExport.h"

class qSlicerImagePositioningModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_IMAGEPOSITIONING_EXPORT qSlicerImagePositioningModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerImagePositioningModuleWidget(QWidget *parent=0);
  virtual ~qSlicerImagePositioningModuleWidget();

public slots:

  void onSetViewClicked();
  void onSetCustomLayoutClicked();
  void onHorizontalImageClicked();
  void onVerticalImageClicked();
  void onDrrImageNodeChanged(vtkMRMLNode* drrImageNode);
  void onXrayImageNodeChanged(vtkMRMLNode* xrayImageNode);
//  void onDrrOpacityChanged(double* opacity);
  void onXrayOpacityChanged(double opacity);
  void setSliceOrientation();
protected:
  QScopedPointer<qSlicerImagePositioningModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerImagePositioningModuleWidget);
  Q_DISABLE_COPY(qSlicerImagePositioningModuleWidget);
};

#endif
