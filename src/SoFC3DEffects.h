#pragma once

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoSceneTexture2.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCallback.h>

namespace Gui
{

class /*GuiExport*/ SoFC3DEffects : public SoSeparator
{
    //typedef SoSeparator inherited;
    using inherited = SoGroup;
    SO_NODE_HEADER(Gui::SoFC3DEffects);

public:
    static void initClass();
    //static void finish();
    SoFC3DEffects();

    SoSFString documentName;
    SoSFString objectName;
    SoSFString subElementName;
    SoSFBool enableBasePlaneShadow;
    SoSFBool enableAmbientOclusion;
    SoSFBool enableShadows;

    //void doAction(SoAction* action) override;
    //virtual void GLRender(SoGLRenderAction* action) override;
    void setScene(SoNode* scene);

    //void handleEvent(SoHandleEventAction* action) override;
    //void GLRenderBelowPath(SoGLRenderAction* action) override;
    //void GLRenderInPath(SoGLRenderAction* action) override;
    //static  void turnOffCurrentHighlight(SoGLRenderAction* action);
    void updateGeometry();

protected:

    void createScene();
    void doDebug();
    void updateBoundingBox();
    void updateCameraView();
    void updatePlaneCoords();
    void createShadow();
    void createBlurShader(SoShaderProgram* prog, bool isHorizontal);
    void createShadowBlur(SoSceneTexture2* fromTex, SoSceneTexture2* toTex, SoShaderProgram* prog);
    void createShadowPlane(SoSceneTexture2* texture);

    ~SoFC3DEffects() override;

    SoNode* Scene;
    SoGroup* ShadowPlane;
    SoSeparator* BaseShadowScene;
    SoSceneTexture2* BaseShadowTexture;
    SoSceneTexture2* BlurPass1Texture;
    SoSceneTexture2* BlurPass2Texture;
    SbBox3f BoundingBox;
    SoShaderProgram* ShaderProgHoriz;
    SoShaderProgram* ShaderProgVert;
    SoOrthographicCamera* OrthoCam;
    SoCoordinate3* ShadowPlaneCoords;
    SoCallback* UpdateCallback;

};

}

