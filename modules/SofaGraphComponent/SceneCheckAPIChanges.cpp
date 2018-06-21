/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2018 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <sofa/version.h>
#include <string>
#include <sofa/core/objectmodel/Base.h>
using sofa::core::objectmodel::Base ;
using sofa::core::objectmodel::BaseObjectDescription ;

#include <sofa/core/ObjectFactory.h>
using sofa::core::ObjectFactory ;

#include <sofa/simulation/Visitor.h>
#include <sofa/helper/system/PluginManager.h>
#include <sofa/helper/system/FileRepository.h>

#include <sofa/helper/ComponentChange.h>
using sofa::helper::lifecycle::ComponentChange;
using sofa::helper::lifecycle::deprecatedComponents;



#include "SceneCheckAPIChanges.h"
#include "RequiredPlugin.h"

#include "APIVersion.h"
using sofa::component::APIVersion ;

namespace sofa
{
namespace simulation
{
namespace _scenecheckapichange_
{

SceneCheckAPIChange::SceneCheckAPIChange()
{
    installDefaultChangeSets() ;
}

SceneCheckAPIChange::~SceneCheckAPIChange()
{

}

const std::string SceneCheckAPIChange::getName()
{
    return "SceneCheckAPIChange";
}

const std::string SceneCheckAPIChange::getDesc()
{
    return "Check for each component that the behavior have not changed since reference version of sofa.";
}

void SceneCheckAPIChange::doInit(Node* node)
{
    std::stringstream version;
    version << SOFA_VERSION / 10000 << "." << SOFA_VERSION / 100 % 100;
    m_currentApiLevel = version.str();

    APIVersion* apiversion {nullptr} ;
    /// 1. Find if there is an APIVersion component in the scene. If there is none, warn the user and set
    /// the version to 17.06 (the last version before it was introduced). If there is one...use
    /// this component to request the API version requested by the scene.
    node->getTreeObject(apiversion) ;
    if(!apiversion)
    {
        msg_info("SceneCheckAPIChange") << "The 'APIVersion' directive is missing in the current scene. Switching to the default APIVersion level '"<< m_selectedApiLevel <<"' " ;
    }
    else
    {
        m_selectedApiLevel = apiversion->getApiLevel() ;
    }
}

void SceneCheckAPIChange::doPrintSummary()
{
    // Alias use summary
    if ( ! this->m_componentsCreatedUsingAlias.empty() )
    {
        std::stringstream usingAliasesWarning;
        usingAliasesWarning << "This scene is using aliases. Aliases are dangerous, use with caution." << msgendl;
        for (auto i : this->m_componentsCreatedUsingAlias)
        {
            if (i.second.size() > 1)
                usingAliasesWarning << "  - " << i.first << " have been created using the aliases ";
            else
                usingAliasesWarning << "  - " << i.first << " has been created using the alias ";

            bool first = true;
            for (std::string &alias : i.second)
            {
                if (first)
                    usingAliasesWarning << "\"" << alias << "\"";
                else
                    usingAliasesWarning << ", \"" << alias << "\"";

                first = false;
            }
            usingAliasesWarning << "." << msgendl;
        }
        msg_warning("SceneCheckAPIChanges") << usingAliasesWarning.str();
    }
}

void SceneCheckAPIChange::doCheckOn(Node* node)
{
    if(node==nullptr)
        return ;

    for (auto& object : node->object )
    {
        if(m_selectedApiLevel != m_currentApiLevel && m_changesets.find(m_selectedApiLevel) != m_changesets.end())
        {
            for(auto& hook : m_changesets[m_selectedApiLevel])
            {
                hook(object.get());
            }
        }
    }
}


void SceneCheckAPIChange::installDefaultChangeSets()
{
    addHookInChangeSet("17.06", [](Base* o){
        if(o->getClassName() == "BoxStiffSpringForceField" )
            msg_warning(o) << "BoxStiffSpringForceField have changed since 17.06. To use the old behavior you need to set parameter 'forceOldBehavior=true'" ;
    }) ;

    addHookInChangeSet("17.06", [this](Base* o){
        if( deprecatedComponents.find( o->getClassName() ) != deprecatedComponents.end() )
        {
            msg_deprecated(o) << deprecatedComponents.at(o->getClassName()).getMessage();
        }
    }) ;

    for(auto i : this->m_componentsCreatedUsingAlias)
    {
        std::string aliases;
        for (std::string &alias : i.second)
        {
            aliases += " " + alias;
        }
        msg_warning(i.first) << "Using the aliases: " << aliases;
    }

    /// Add a callback to be n
    ObjectFactory::getInstance()->setCallback([this](Base* o, BaseObjectDescription *arg) {
        if (o->getClassName() != arg->getAttribute("type", "") ) {
            std::string alias = arg->getAttribute("type", "");

            std::vector<std::string> v = this->m_componentsCreatedUsingAlias[o->getClassName()];
            if ( std::find(v.begin(), v.end(), alias) == v.end() )
            {
                this->m_componentsCreatedUsingAlias[o->getClassName()].push_back(alias);
            }
        }
    });
}

void SceneCheckAPIChange::addHookInChangeSet(const std::string& version, ChangeSetHookFunction fct)
{
    m_changesets[version].push_back(fct) ;
}


} // _scenecheckapichange_

} // namespace simulation

} // namespace sofa

