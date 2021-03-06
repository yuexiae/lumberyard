/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "AnimGraphOptions.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <EMotionStudio/EMStudioSDK/Source/PluginOptionsBus.h>

#include <QSettings>

namespace EMStudio
{
    const char* AnimGraphOptions::s_graphAnimationOptionName = "useGraphAnimation";
    const char* AnimGraphOptions::s_showFPSOptionName = "showFPS";

    AnimGraphOptions::AnimGraphOptions()
        : m_graphAnimation(true)
        , m_showFPS(false)
    {}

    AnimGraphOptions& AnimGraphOptions::operator=(const AnimGraphOptions& other)
    {
        SetGraphAnimation(other.GetGraphAnimation());
        SetShowFPS(other.GetShowFPS());

        return *this;
    }

    void AnimGraphOptions::Save(QSettings* settings)
    {
        settings->setValue(s_graphAnimationOptionName, m_graphAnimation);
        settings->setValue(s_showFPSOptionName, m_showFPS);
    }

    AnimGraphOptions AnimGraphOptions::Load(QSettings* settings)
    {
        AnimGraphOptions options;

        QVariant tmpVariant = settings->value(s_graphAnimationOptionName);
        if (!tmpVariant.isNull())
        {
            options.m_graphAnimation = tmpVariant.toBool();
        }
        tmpVariant = settings->value(s_showFPSOptionName);
        if (!tmpVariant.isNull())
        {
            options.m_showFPS = tmpVariant.toBool();
        }

        return options;
    }

    void AnimGraphOptions::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (!serializeContext)
        {
            return;
        }

        serializeContext->Class<AnimGraphOptions>()
            ->Version(1)
            ->Field(s_graphAnimationOptionName, &AnimGraphOptions::m_graphAnimation)
            ->Field(s_showFPSOptionName, &AnimGraphOptions::m_showFPS)
            ;

        AZ::EditContext* editContext = serializeContext->GetEditContext();
        if (!editContext)
        {
            return;
        }

        editContext->Class<AnimGraphOptions>("Anim graph plugin properties", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
            ->DataElement(AZ::Edit::UIHandlers::Default, &AnimGraphOptions::m_graphAnimation, "Graph animation", "")
                ->Attribute(AZ::Edit::Attributes::ChangeNotify, &AnimGraphOptions::OnGraphAnimationChangedCallback)
            ->DataElement(AZ::Edit::UIHandlers::Default, &AnimGraphOptions::m_showFPS, "Show FPS", "")
                ->Attribute(AZ::Edit::Attributes::ChangeNotify, &AnimGraphOptions::OnShowFPSChangedCallback)
            ;
    }

    void AnimGraphOptions::SetGraphAnimation(bool graphAnimation)
    {
        if (graphAnimation != m_graphAnimation)
        {
            m_graphAnimation = graphAnimation;
            OnGraphAnimationChangedCallback();
        }
    }

    void AnimGraphOptions::SetShowFPS(bool showFPS)
    {
        if (showFPS != m_showFPS)
        {
            m_showFPS = showFPS;
            OnShowFPSChangedCallback();
        }
    }

    void AnimGraphOptions::OnGraphAnimationChangedCallback() const
    {
        PluginOptionsNotificationsBus::Event(s_graphAnimationOptionName, &PluginOptionsNotificationsBus::Events::OnOptionChanged, s_graphAnimationOptionName);
    }

    void AnimGraphOptions::OnShowFPSChangedCallback() const
    {
        PluginOptionsNotificationsBus::Event(s_showFPSOptionName, &PluginOptionsNotificationsBus::Events::OnOptionChanged, s_showFPSOptionName);
    }

} // namespace EMStudio

