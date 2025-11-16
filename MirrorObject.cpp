#include "stdafx.h"
#include "MirrorObject.h"
#include "Mesh.h" // Include Mesh.h to access CTexturedRectMesh

CMirrorObject::CMirrorObject() : CGameObject(1, 1) // Assuming 1 mesh and 1 material for the mirror
{
}

CMirrorObject::~CMirrorObject()
{
}

void CMirrorObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
    CGameObject::UpdateTransform(pxmf4x4Parent);

    // Calculate mirror plane here
    if (m_ppMeshes && m_ppMeshes[0])
    {
        CTexturedRectMesh* pMirrorMesh = dynamic_cast<CTexturedRectMesh*>(m_ppMeshes[0]);
        if (pMirrorMesh && pMirrorMesh->m_pxmf3Positions && pMirrorMesh->m_nVertices >= 3)
        {
            // Get three vertices from the mesh in local space
            XMFLOAT3 p0_local = pMirrorMesh->m_pxmf3Positions[0];
            XMFLOAT3 p1_local = pMirrorMesh->m_pxmf3Positions[1];
            XMFLOAT3 p2_local = pMirrorMesh->m_pxmf3Positions[2];

            // Transform local vertices to world space
            XMMATRIX worldMatrix = XMLoadFloat4x4(&m_xmf4x4World);
            XMVECTOR v0_world = XMVector3TransformCoord(XMLoadFloat3(&p0_local), worldMatrix);
            XMVECTOR v1_world = XMVector3TransformCoord(XMLoadFloat3(&p1_local), worldMatrix);
            XMVECTOR v2_world = XMVector3TransformCoord(XMLoadFloat3(&p2_local), worldMatrix);

            // Calculate the plane
            XMVECTOR v01 = XMVectorSubtract(v1_world, v0_world);
            XMVECTOR v02 = XMVectorSubtract(v2_world, v0_world);
            XMVECTOR n = XMVector3Normalize(XMVector3Cross(v01, v02));

            float d = -XMVectorGetX(XMVector3Dot(n, v0_world));
            XMStoreFloat4(&m_xmf4MirrorPlane, XMVectorSetW(n, d));
        }
    }
}