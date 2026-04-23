// Qt includes
#include <QDebug>

#include <vtkTransform.h>
// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
//#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>


#include "vtkMRMLImagePositioningNode.h"

//------------------------------------------------------------------------------
namespace
{
    static const char* TRANSFORM_NODE_REFERENCE_ROLE = "transformNodeRef";
    static const char* DRR_NODE_REFERENCE_ROLE = "drrNodeRef";
    static const char* XRAY_NODE_REFERENCE_ROLE = "xrayNodeRef";

}

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLImagePositioningNode);

vtkMRMLImagePositioningNode::vtkMRMLImagePositioningNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLImagePositioningNode::~vtkMRMLImagePositioningNode()
{
    this->SetAndObserveTransformNode(nullptr);
    this->SetAndObserveDrrNode(nullptr);
    this->SetAndObserveXrayNode(nullptr);
}

void vtkMRMLImagePositioningNode::WriteXML(ostream& of, int nIndent)
{
    Superclass::WriteXML(of, nIndent);

    // Write all MRML node attributes into output stream
    vtkMRMLWriteXMLBeginMacro(of);


     // add new parameters here
    vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLImagePositioningNode::ReadXMLAttributes(const char** atts)
{
    int disabledModify = this->StartModify();
    vtkMRMLNode::ReadXMLAttributes(atts);

    vtkMRMLReadXMLBeginMacro(atts);



      // add new parameters here
    vtkMRMLReadXMLEndMacro();

    this->EndModify(disabledModify);

    // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLImagePositioningNode::Copy(vtkMRMLNode* anode)
{
    int disabledModify = this->StartModify();

    Superclass::Copy(anode);

    vtkMRMLImagePositioningNode* node = vtkMRMLImagePositioningNode::SafeDownCast(anode);
    if (!node)
    {
        return;
    }

    this->DisableModifiedEventOn();

    vtkMRMLCopyBeginMacro(node);

     // add new parameters here
    vtkMRMLCopyEndMacro();

    this->EndModify(disabledModify);

    this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLImagePositioningNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
    MRMLNodeModifyBlocker blocker(this);
    Superclass::CopyContent(anode, deepCopy);

    vtkMRMLImagePositioningNode* node = vtkMRMLImagePositioningNode::SafeDownCast(anode);
    if (!node)
    {
        return;
    }

    vtkMRMLCopyBeginMacro(node);


     // add new parameters here
    vtkMRMLCopyEndMacro();
}

void vtkMRMLImagePositioningNode::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);

    vtkMRMLPrintBeginMacro(os, indent);

    os << indent << "TransformNode: " << (this->GetTransformNode() ? this->GetTransformNode()->GetName() : "null") << "\n";
    os << indent << "DrrNode: " << (this->GetDrrNode() ? this->GetDrrNode()->GetName() : "null") << "\n";
    os << indent << "XrayNode: " << (this->GetXrayNode() ? this->GetXrayNode()->GetName() : "null") << "\n";

    // add new parameters here
    vtkMRMLPrintEndMacro();

}
/*
void vtkMRMLImagePositioningNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene");
    return;
  }
  if (this->Scene->IsBatchProcessing())
  {
    return;
  }

}
*/

vtkMRMLLinearTransformNode* vtkMRMLImagePositioningNode::GetTransformNode()
{
    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(TRANSFORM_NODE_REFERENCE_ROLE));
}

vtkMRMLScalarVolumeNode* vtkMRMLImagePositioningNode::GetDrrNode()
{
    return vtkMRMLScalarVolumeNode::SafeDownCast(this->GetNodeReference(DRR_NODE_REFERENCE_ROLE));
}

vtkMRMLScalarVolumeNode* vtkMRMLImagePositioningNode::GetXrayNode()
{
    return vtkMRMLScalarVolumeNode::SafeDownCast(this->GetNodeReference(XRAY_NODE_REFERENCE_ROLE));
}


void vtkMRMLImagePositioningNode::SetAndObserveTransformNode(vtkMRMLLinearTransformNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }

    this->SetNodeReferenceID(TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));

    //  this->Modified();
}

void vtkMRMLImagePositioningNode::SetAndObserveDrrNode(vtkMRMLScalarVolumeNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }

    this->SetNodeReferenceID(DRR_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));

    // this->FiducialNode = node;

    // this->Modified();
}
