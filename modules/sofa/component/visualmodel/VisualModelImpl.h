/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_VISUALMODEL_VISUALMODELIMPL_H
#define SOFA_COMPONENT_VISUALMODEL_VISUALMODELIMPL_H

#include <sofa/core/State.h>
#include <sofa/core/VisualModel.h>
#include <sofa/core/objectmodel/DataFileName.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/defaulttype/Vec.h>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/helper/io/Mesh.h>

#include <sofa/component/component.h>

#include <map>
#include <string>


namespace sofa
{

namespace component
{

namespace visualmodel
{

using namespace sofa::defaulttype;
using namespace sofa::core::loader;


class RigidState : public core::State< Rigid3fTypes >
{
public:
    VecCoord xforms;
    bool xformsModified;

    RigidState()
        : xformsModified(false)
    {
    }

    virtual void resize(int vsize) { xformsModified = true; xforms.resize( vsize); }
    virtual int getSize() const { return 0; }

    const VecCoord* getX()  const { return &xforms; }
    const VecDeriv* getV()  const { return NULL; }

    VecCoord* getX()  { xformsModified = true; return &xforms;   }
    VecDeriv* getV()  { return NULL; }

    const VecCoord* getRigidX()  const { return getX(); }
    VecCoord* getRigidX()  { return getX(); }

    virtual       Data<VecCoord>* write(     core::VecCoordId /* v */) { return NULL; }
    virtual const Data<VecCoord>*  read(core::ConstVecCoordId /* v */) const { return NULL; }

    virtual       Data<VecDeriv>* write(     core::VecDerivId /* v */) { return NULL; }
    virtual const Data<VecDeriv>*  read(core::ConstVecDerivId /* v */) const { return NULL; }

    virtual       Data<MatrixDeriv>* write(     core::MatrixDerivId /* v */) { return NULL; }
    virtual const Data<MatrixDeriv>*  read(core::ConstMatrixDerivId /* v */) const {  return NULL; }
};

class ExtVec3fState : public core::State< ExtVec3fTypes >
{
public:
    Data< ResizableExtVector<Coord> > m_positions;
    Data< ResizableExtVector<Coord> > m_restPositions;
    Data< ResizableExtVector<Deriv> > m_vnormals;
    bool modified; ///< True if input vertices modified since last rendering

    ExtVec3fState()
        : m_positions(initData(&m_positions, "position", "Vertices coordinates"))
        , m_restPositions(initData(&m_restPositions, "restPosition", "Vertices rest coordinates"))
        , m_vnormals (initData (&m_vnormals, "normals", "Normals of the model"))
        , modified(false)
    {
        m_positions.setGroup("Vector");
        m_restPositions.setGroup("Vector");
        m_vnormals.setGroup("Vector");
        addAlias(&m_vnormals,"normal");
    }

    virtual void resize(int vsize)
    {
        modified = true;
        helper::WriteAccessor< Data<ResizableExtVector<Coord> > > positions = m_positions;
        helper::WriteAccessor< Data<ResizableExtVector<Coord> > > restPositions = m_restPositions;
        helper::WriteAccessor< Data<ResizableExtVector<Deriv> > > normals = m_vnormals;

        positions.resize(vsize);
        restPositions.resize(vsize);
        normals.resize(vsize);
    }

    virtual int getSize() const { return m_positions.getValue().size(); }

    //State API
    virtual       Data<VecCoord>* write(     core::VecCoordId  v )
    {
        modified = true;

        if( v == core::VecCoordId::position() )
            return &m_positions;
        if( v == core::VecCoordId::restPosition() )
            return &m_restPositions;

        return NULL;
    }
    virtual const Data<VecCoord>*  read(core::ConstVecCoordId  v )  const
    {
        if( v == core::VecCoordId::position() )
            return &m_positions;
        if( v == core::VecCoordId::restPosition() )
            return &m_restPositions;

        return NULL;
    }

    virtual Data<VecDeriv>*	write(core::VecDerivId v )
    {
        if( v == core::VecDerivId::normal() )
            return &m_vnormals;

        return NULL;
    }

    virtual const Data<VecDeriv>* read(core::ConstVecDerivId v ) const
    {
        if( v == core::VecDerivId::normal() )
            return &m_vnormals;

        return NULL;
    }

    virtual       Data<MatrixDeriv>*	write(     core::MatrixDerivId /* v */) { return NULL; }
    virtual const Data<MatrixDeriv>*	read(core::ConstMatrixDerivId /* v */) const {  return NULL; }
};

/**
 *  \brief Abstract class which implements partially VisualModel.
 *
 *  This class implemented all non-hardware (i.e OpenGL or DirectX)
 *  specific functions for rendering. It takes a 3D model (basically a .OBJ model)
 *  and apply transformations on it.
 *  At the moment, it is only implemented by OglModel for OpenGL systems.
 *
 */

class SOFA_COMPONENT_VISUALMODEL_API VisualModelImpl : public core::VisualModel, public ExtVec3fState //, public RigidState
{
public:
    SOFA_CLASS2(VisualModelImpl, core::VisualModel, ExtVec3fState);
//    SOFA_CLASS3(VisualModelImpl, core::VisualModel, ExtVec3fState , RigidState);

    typedef Vec<2, float> TexCoord;

    typedef sofa::core::topology::BaseMeshTopology::Triangle Triangle;
    typedef sofa::core::topology::BaseMeshTopology::Quad Quad;

protected:

    typedef ExtVec3fTypes DataTypes;
    typedef DataTypes::Real Real;
    typedef DataTypes::Coord Coord;
    typedef DataTypes::VecCoord VecCoord;
    typedef DataTypes::Deriv Deriv;
    typedef DataTypes::VecDeriv VecDeriv;

    //ResizableExtVector<Coord>* inputVertices;

    bool useTopology; ///< True if list of facets should be taken from the attached topology
    int lastMeshRev; ///< Time stamps from the last time the mesh was updated from the topology
    bool castShadow; ///< True if object cast shadows

    sofa::core::topology::BaseMeshTopology* m_topology;

    Data<bool> m_useNormals; ///< True if normals should be read from file
    Data<bool> m_updateNormals; ///< True if normals should be updated at each iteration
    Data<bool> m_computeTangents; ///< True if tangents should be computed at startup
    Data<bool> m_updateTangents; ///< True if tangents should be updated at each iteration

    Data< ResizableExtVector< Coord > > m_vertices;
    Data< ResizableExtVector< TexCoord > > m_vtexcoords;
    Data< ResizableExtVector< Coord > > m_vtangents;
    Data< ResizableExtVector< Coord > > m_vbitangents;
    Data< ResizableExtVector< Triangle > > m_triangles;
    Data< ResizableExtVector< Quad > > m_quads;

    /// If vertices have multiple normals/texcoords, then we need to separate them
    /// This vector store which input position is used for each vertice
    /// If it is empty then each vertex correspond to one position
    ResizableExtVector<int> vertPosIdx;

    /// Similarly this vector store which input normal is used for each vertice
    /// If it is empty then each vertex correspond to one normal
    ResizableExtVector<int> vertNormIdx;

    /// Rendering method.
    virtual void internalDraw(bool /*transparent*/) {};

public:

    sofa::core::objectmodel::DataFileName fileMesh;
    sofa::core::objectmodel::DataFileName texturename;

    /// @name Initial transformation attributes
    /// @{
    Data< Vector3 > m_translation;
    Data< Vector3 > m_rotation;
    Data< Vector3 > m_scale;

    Data< TexCoord > m_scaleTex;
    Data< TexCoord > m_translationTex;

    void applyTranslation(const double dx, const double dy, const double dz);

    /// Apply Rotation from Euler angles (in degree!)
    void applyRotation (const double rx, const double ry, const double rz);

    void applyRotation(const Quat q);

    void applyScale(const double sx, const double sy, const double sz);

    virtual void applyUVTransformation();

    void applyUVTranslation(const double dU, const double dV);

    void applyUVScale(const double su, const double sv);

    void setTranslation(double dx, double dy, double dz)
    {
        m_translation.setValue(Vector3(dx,dy,dz));
    };

    void setRotation(double rx, double ry, double rz)
    {
        m_rotation.setValue(Vector3(rx,ry,rz));
    };

    void setScale(double sx, double sy, double sz)
    {
        m_scale.setValue(Vector3(sx,sy,sz));
    };
    /// @}

    Vec3f bbox[2];
    Data< Material > material;
#ifdef SOFA_SMP
    sofa::core::loader::Material originalMaterial;
    bool previousProcessorColor;
#endif
    Data< bool > putOnlyTexCoords;
    Data< bool > srgbTexturing;

    class FaceGroup
    {
    public:
        int t0, nbt;
        int q0, nbq;
        std::string materialName;
        std::string groupName;
        int materialId;
        FaceGroup() : t0(0), nbt(0), q0(0), nbq(0), materialName("defaultMaterial"), groupName("defaultGroup"), materialId(-1) {}
        inline friend std::ostream& operator << (std::ostream& out, const FaceGroup &g)
        {
            out << g.groupName << " " << g.materialName << " " << g.materialId << " " << g.t0 << " " << g.nbt << " " << g.q0 << " " << g.nbq;
            return out;
        }
        inline friend std::istream& operator >> (std::istream& in, FaceGroup &g)
        {

            in >> g.groupName >> g.materialName >> g.materialId >> g.t0 >> g.nbt >> g.q0 >> g.nbq;
            return in;
        }
    };

    Data< helper::vector<Material> > materials;
    Data< helper::vector<FaceGroup> > groups;

    /// Default constructor.
    VisualModelImpl();

    /// Default destructor.
    ~VisualModelImpl();

    void parse(core::objectmodel::BaseObjectDescription* arg);

    bool hasTransparent();
    bool hasOpaque();

    void drawVisual();
    void drawTransparent();
    void drawShadow();

    virtual bool loadTextures() {return false;}
    virtual bool loadTexture(const std::string& /*filename*/) { return false; }

    bool load(const std::string& filename, const std::string& loader, const std::string& textureName);

    void flipFaces();

    void setFilename(std::string s)
    {
        fileMesh.setValue(s);
    };

    std::string getFilename() {return fileMesh.getValue();}

    void setColor(float r, float g, float b, float a);

    void setColor(std::string color);

    void setUseNormals(bool val)
    {
        m_useNormals.setValue(val);
    };

    bool getUseNormals() const
    {
        return m_useNormals.getValue();
    };

    void setCastShadow(bool val)
    {
        castShadow = val;
    };

    bool getCastShadow() const
    {
        return castShadow;
    };

    void setMesh(helper::io::Mesh &m, bool tex=false);

    bool isUsingTopology() const
    {
        return useTopology;
    };

    const ResizableExtVector<Coord>& getVertices() const
    {
        if (!vertPosIdx.empty())
        {
            // Splitted vertices for multiple texture or normal coordinates per vertex.
            return m_vertices.getValue();
        }

        return m_positions.getValue();
    };

    const ResizableExtVector<Deriv>& getVnormals() const
    {
        return m_vnormals.getValue();
    };

    const ResizableExtVector<TexCoord>& getVtexcoords() const
    {
        return m_vtexcoords.getValue();
    };

    const ResizableExtVector<Coord>& getVtangents() const
    {
        return m_vtangents.getValue();
    };

    const ResizableExtVector<Coord>& getVbitangents() const
    {
        return m_vbitangents.getValue();
    };

    const ResizableExtVector<Triangle>& getTriangles() const
    {
        return m_triangles.getValue();
    };

    const ResizableExtVector<Quad>& getQuads() const
    {
        return m_quads.getValue();
    };

    void setVertices(ResizableExtVector<Coord> * x)
    {
        m_vertices.setValue(*x);
    };

    void setVnormals(ResizableExtVector<Deriv> * vn)
    {
        m_vnormals.setValue(*vn);
    };

    void setVtexcoords(ResizableExtVector<TexCoord> * vt)
    {
        m_vtexcoords.setValue(*vt);
    };

    void setVtangents(ResizableExtVector<Coord> * v)
    {
        m_vtangents.setValue(*v);
    };

    void setVbitangents(ResizableExtVector<Coord> * v)
    {
        m_vbitangents.setValue(*v);
    };

    void setTriangles(ResizableExtVector<Triangle> * t)
    {
        m_triangles.setValue(*t);
    };

    void setQuads(ResizableExtVector<Quad> * q)
    {
        m_quads.setValue(*q);
    };

    virtual void computePositions();
    virtual void computeMesh();
    virtual void computeNormals();
    virtual void computeTangents();
    virtual void computeBBox();

    virtual void updateBuffers() {};

    virtual void updateVisual();

    // Handle topological changes
    virtual void handleTopologyChange();

    void init();

    void initVisual();

    bool addBBox(double* minBBox, double* maxBBox);

    /// Append this mesh to an OBJ format stream.
    /// The number of vertices position, normal, and texture coordinates already written is given as parameters
    /// This method should update them
    virtual void exportOBJ(std::string name, std::ostream* out, std::ostream* mtl, int& vindex, int& nindex, int& tindex);

    virtual std::string getTemplateName() const
    {
        return ExtVec3fState::getTemplateName();
    }

    static std::string templateName(const VisualModelImpl* p = NULL)
    {
        return ExtVec3fState::templateName(p);
    }

    /// Utility method to compute tangent from vertices and texture coordinates.
    static Coord computeTangent(const Coord &v1, const Coord &v2, const Coord &v3,
            const TexCoord &t1, const TexCoord &t2, const TexCoord &t3);

    /// Utility method to compute bitangent from vertices and texture coordinates.
    static Coord computeBitangent(const Coord &v1, const Coord &v2, const Coord &v3,
            const TexCoord &t1, const TexCoord &t2, const TexCoord &t3);

    /// Temporary added here from RigidState deprecated inheritance
    Rigid3fTypes::VecCoord xforms;
    bool xformsModified;
};

} // namespace visualmodel

} // namespace component

} // namespace sofa

#endif
