#include "SoFC3DEffects.h"
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoShaderParameter.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoFragmentShader.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/nodes/SoEnvironment.h>
#include <Inventor/nodes/SoDepthBuffer.h>
#include <Inventor/actions/SoGLRenderAction.h>


using namespace Gui;

// *************************************************************************
extern char const TxtVertexShader[];
extern char const TxtShadowFragmentShader[];

#define SHADOW_TEX_W  256
#define SHADOW_TEX_H  256

static void UpdateCallbackFunc(void* data, SoAction* action) {
    if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
        SoFC3DEffects* effects = (SoFC3DEffects*)data;
        effects->updateGeometry();
        //SoCacheElement::invalidate(action->getState());
    }
}

SO_NODE_SOURCE(SoFC3DEffects)

SoFC3DEffects::SoFC3DEffects()
{
    //SO_NODE_INTERNAL_CONSTRUCTOR(SoGeoSeparator);

    SO_NODE_CONSTRUCTOR(SoFC3DEffects);

    SO_NODE_ADD_FIELD(documentName, (""));
    SO_NODE_ADD_FIELD(objectName, (""));
    SO_NODE_ADD_FIELD(subElementName, (""));
    SO_NODE_ADD_FIELD(enableBasePlaneShadow, (false));
    SO_NODE_ADD_FIELD(enableAmbientOclusion, (false));
    SO_NODE_ADD_FIELD(enableShadows, (false));

    createScene();
}

void
SoFC3DEffects::initClass()
{
    SO_NODE_INIT_CLASS(SoFC3DEffects, SoSeparator, "Separator");
}


void SoFC3DEffects::setScene(SoNode* scene)
{
    Scene = scene;
    updateGeometry();
    BaseShadowScene->removeAllChildren();
    BaseShadowScene->addChild(OrthoCam);
    BaseShadowScene->addChild(Scene);
}


void Gui::SoFC3DEffects::doDebug()
{
    enableBasePlaneShadow = true;
}

void Gui::SoFC3DEffects::createScene()
{
    createShadow();

    UpdateCallback = new SoCallback;
    UpdateCallback->setCallback(UpdateCallbackFunc, this);
    addChild(UpdateCallback);

    // blur shadow pass 1
    ShaderProgHoriz = new SoShaderProgram;
    createBlurShader(ShaderProgHoriz, true);
    BlurPass1Texture = new SoSceneTexture2;
    createShadowBlur(BaseShadowTexture, BlurPass1Texture, ShaderProgHoriz);

    // blur shadow pass 2
    ShaderProgVert = new SoShaderProgram;
    createBlurShader(ShaderProgVert, false);
    BlurPass2Texture = new SoSceneTexture2;
    createShadowBlur(BlurPass1Texture, BlurPass2Texture, ShaderProgVert);

    // a transparent plane under the scene to accept the shadow
    createShadowPlane(BlurPass2Texture);
    addChild(ShadowPlane);
}

void Gui::SoFC3DEffects::updateBoundingBox()
{
    SbViewportRegion myViewport;
    SoGetBoundingBoxAction object_bbox(myViewport);
    object_bbox.apply(Scene);
    BoundingBox = object_bbox.getBoundingBox();

    float sizex, sizey, sizez;
    BoundingBox.getSize(sizex, sizey, sizez);
    float pad = sizex > sizey ? sizex : sizey;
    pad *= 0.2;

    SbVec3f& minp = BoundingBox.getMin();
    SbVec3f& maxp = BoundingBox.getMax();
    BoundingBox.setBounds(minp[0] - pad, minp[1] - pad, minp[2] - pad, maxp[0] + pad, maxp[1] + pad, maxp[2] + pad);
}

void Gui::SoFC3DEffects::updateCameraView()
{
    float sizex, sizey, sizez;
    BoundingBox.getSize(sizex, sizey, sizez);
    SbVec3f cent = BoundingBox.getCenter();
    float maxy = BoundingBox.getMax()[1];
    OrthoCam->height = sizex;
    OrthoCam->aspectRatio = sizex / sizez;
    OrthoCam->nearDistance = 0;
    OrthoCam->farDistance = sizey;
    OrthoCam->viewportMapping = OrthoCam->LEAVE_ALONE;
    OrthoCam->position = SbVec3f(cent[0], maxy, cent[2]);
    OrthoCam->pointAt(SbVec3f(cent[0], cent[1], cent[2]), SbVec3f(0, 0, -1));
}

void Gui::SoFC3DEffects::updatePlaneCoords()
{
    SbVec3f& minp = BoundingBox.getMin();
    SbVec3f& maxp = BoundingBox.getMax();
    float y = minp[1];

    static float vertices[4][3] = {
        { minp[0], y, maxp[2]},
        { maxp[0], y, maxp[2]},
        { maxp[0], y, minp[2]},
        { minp[0], y, minp[2]}
    };
    ShadowPlaneCoords->point.setValues(0, 4, vertices);
}

void Gui::SoFC3DEffects::updateGeometry()
{
    updateBoundingBox();
    updateCameraView();
    updatePlaneCoords();
}

void SoFC3DEffects::createShadow()
{
    BaseShadowTexture = new SoSceneTexture2;
    BaseShadowScene = new SoSeparator;

    OrthoCam = new SoOrthographicCamera;

    BaseShadowTexture->scene = BaseShadowScene;
    BaseShadowTexture->size.setValue(SHADOW_TEX_W, SHADOW_TEX_H);
    BaseShadowTexture->model = SoTexture2::DECAL;
}

void SoFC3DEffects::createBlurShader(SoShaderProgram* prog, bool isHorizontal)
{
    SoVertexShader* vert = new SoVertexShader;
    vert->sourceType = SoVertexShader::GLSL_PROGRAM;
    vert->sourceProgram = TxtVertexShader;
    SoFragmentShader* frag = new SoFragmentShader;
    frag->sourceType = SoFragmentShader::GLSL_PROGRAM;
    frag->sourceProgram = TxtShadowFragmentShader;
    SoShaderParameter1i* texSlot = new SoShaderParameter1i;
    texSlot->name = "u_input_texture";
    texSlot->value = 0;
    SoShaderParameter2f* blurDir = new SoShaderParameter2f;
    blurDir->name = "u_direction";
    if (isHorizontal)
        blurDir->value.setValue(1.0 / 256.0, 0);
    else
        blurDir->value.setValue(0, 1.0 / 256.0);
    frag->parameter.set1Value(0, texSlot);
    frag->parameter.set1Value(1, blurDir);
    prog->shaderObject.set1Value(0, vert);
    prog->shaderObject.set1Value(1, frag);
}

void Gui::SoFC3DEffects::createShadowBlur(SoSceneTexture2* fromTex, SoSceneTexture2* toTex, SoShaderProgram* prog)
{
    static float vertices[4][3] = {
        { -1, -1, 0},
        {  1, -1, 0},
        {  1,  1, 0},
        { -1,  1, 0}
    };

    SoGroup* grp = new SoGroup;
    grp->addChild(prog);
    grp->addChild(fromTex);

    SoCoordinate3* coords = new SoCoordinate3;
    coords->point.setValues(0, 4, vertices);
    grp->addChild(coords);

    SoFaceSet* faceSet = new SoFaceSet;
    faceSet->numVertices.set1Value(0, 3);  // One quad with 4 vertices
    grp->addChild(faceSet);

    toTex->scene = grp;
    toTex->size.setValue(256, 256);
    toTex->wrapS = SoTexture2::CLAMP;
    toTex->wrapT = SoTexture2::CLAMP;

}

void Gui::SoFC3DEffects::createShadowPlane(SoSceneTexture2* texture)
{
    static float texCoordVals[4][2] = {
        { 0, 0 },
        { 1, 0 },
        { 1, 1 },
        { 0, 1 }
    };

    ShadowPlane = new SoGroup;

    SoShapeHints* shapeHints = new SoShapeHints;
    shapeHints->shapeType = SoShapeHints::SOLID;
    shapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
    ShadowPlane->addChild(shapeHints);

    SoMaterial* col = new SoMaterial;
    col->transparency.setValue(0.8);
    col->ambientColor.setValue(1.0, 1.0, 1.0);
    //texture->model = SoTexture2::DECAL;

    ShadowPlane->addChild(col);

    ShadowPlaneCoords = new SoCoordinate3;
    ShadowPlane->addChild(ShadowPlaneCoords);

    SoTextureCoordinate2* texCoords = new SoTextureCoordinate2;
    texCoords->point.setValues(0, 4, texCoordVals);
    SoTextureCoordinateBinding* texBinding = new SoTextureCoordinateBinding;
    texBinding->value = SoTextureCoordinateBinding::PER_VERTEX;
    SoComplexity* complexity = new SoComplexity;
    complexity->textureQuality = 1.0;
    ShadowPlane->addChild(complexity);
    ShadowPlane->addChild(texCoords);
    ShadowPlane->addChild(texBinding);
    ShadowPlane->addChild(texture);

    SoFaceSet* faceSet = new SoFaceSet;
    faceSet->numVertices.set1Value(0, 4);  // One quad with 4 vertices
    ShadowPlane->addChild(faceSet);
}

SoFC3DEffects::~SoFC3DEffects()
{
}

char const TxtVertexShader[] = R"(
#version 330

//layout(location = 0) in vec3 aPos;
const vec2 vertices[3] = vec2[3](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

out vec2 texcoord;

void main()
{
    vec2 aPos = vertices[gl_VertexID];
    gl_Position = vec4(aPos, 0.0, 1.0);

    texcoord = 0.5 * aPos + vec2(0.5);
}
)";

char const TxtShadowFragmentShader[] = R"(
#version 330

uniform sampler2D u_input_texture;
uniform vec2 u_direction;

layout (location = 0) out vec4 out_color;

in vec2 texcoord;

const int M = 16;

// sigma = 10
const float coeffs[M + 1] = float[M + 1](
    0.04425662519949865,
    0.044035873841196206,
    0.043380781642569775,
    0.04231065439216247,
    0.040856643282313365,
    0.039060328279673276,
    0.0369716985390341,
    0.03464682117793548,
    0.03214534135442581,
    0.0295279624870386,
    0.02685404941667096,
    0.02417948052890078,
    0.02155484948872149,
    0.019024086115486723,
    0.016623532195728208,
    0.014381474814203989,
    0.012318109844189502
);

void main()
{
    vec4 sum =  coeffs[0] * texture(u_input_texture, texcoord);
    //float sum = coeffs[0] * texture(u_input_texture, texcoord).a;
    //vec2 dir = vec2(1.0 / 256.0,0);
    vec2 dir = u_direction;

    for (int i = 1; i < M; i += 2)
    {
        float w0 = coeffs[i];
        float w1 = coeffs[i + 1];

        float w = w0 + w1;
        float t = w1 / w;

        sum += w * texture(u_input_texture, texcoord + dir * (float(i) + t));
        sum += w * texture(u_input_texture, texcoord - dir * (float(i) + t));
    }

    out_color = sum;
}
)";


