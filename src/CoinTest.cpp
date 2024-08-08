
#include <QApplication>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoPointLight.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoEnvironment.h>
#include <Inventor/nodes/SoDepthBuffer.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoSceneTexture2.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoShaderParameter.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoFragmentShader.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/SoEventManager.h>
#include <Inventor/elements/SoGLDepthBufferElement.h>
#include <Inventor/SoOffscreenRenderer.h>


#include <Quarter/Quarter.h>
#include <Quarter/QuarterWidget.h>

#include "SoFC3DEffects.h"

using namespace SIM::Coin3D::Quarter;
using namespace Gui;


// second; the scheduling changes every 5 seconds (see below):
static void
rotatingSensorCallback(void* data, SoSensor*)
{
    // Rotate an object...
    SoRotation* rot = (SoRotation*)data;
    SbRotation currentRotation = rot->rotation.getValue();
    currentRotation = SbRotation(SbVec3f(1, 0, 0), M_PI / 90.0) *
        currentRotation;
    rot->rotation.setValue(currentRotation);
}

void CreateObject(SoGroup* object, SbBox3f& boundingBox)
{
    SoBaseColor* col = new SoBaseColor;
    col->rgb = SbColor(1, 1, 0);
    object->addChild(col);

    SoPointLight* pl = new SoPointLight;
    pl->location = SbVec3f(0, 10, 2);
    pl->intensity = 1;
    //object->addChild(pl);

    SoRotation* rot = new SoRotation;
    object->addChild(rot);

    SoTimerSensor* rotatingSensor =
        new SoTimerSensor(rotatingSensorCallback, rot);
    rotatingSensor->setInterval(1.0 / 30.0); //scheduled once per second
    rotatingSensor->schedule();

    object->addChild(new SoCone);
}

//void RenderCallbackFunc(void* data, SoAction* action) {
//    if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
//        sRenderCallbackData* renderData = (sRenderCallbackData*)data;
//        UpdateShadowTexture(renderData->renderer, renderData->root, renderData->texture,
//            renderData->textureWidth, renderData->textureHeight);
//        //SoCacheElement::invalidate(action->getState());
//    }
//}


void CreateBackground(SoSeparator* bgnd)
{
    //SbViewportRegion myRegion(viewer->width(), viewer->height());
    SoDepthBuffer* dbuff = new SoDepthBuffer;
    dbuff->write = false;
    bgnd->addChild(dbuff);


    SoOrthographicCamera* ocam = new SoOrthographicCamera;
    float zbg = 2;
    ocam->height = 2;
    ocam->aspectRatio = 1;
    ocam->nearDistance = 1;
    ocam->farDistance = zbg + 1;
    ocam->viewportMapping = ocam->LEAVE_ALONE;
    ocam->pointAt(SbVec3f(0, 0, 0));
    ocam->position = SbVec3f(0, 0, 9);
    //ocam->viewAll(root, myRegion);


    bgnd->addChild(ocam);

    static float vertices[4][3] = {
        { -1, -1, zbg }, { 1, -1, zbg }, { 1, 1, zbg }, { -1, 1, zbg }
    };
    static float colors[4][3] = {
        { 0.58f, 0.58f, 0.66f }, { 0.58f,0.58f, 0.66f }, { 0.2f, 0.2f, 0.4f }, { 0.2f, 0.2f, 0.4f }
    };

    SoCoordinate3* myCoords = new SoCoordinate3;
    myCoords->point.setValues(0, 4, vertices);
    bgnd->addChild(myCoords);

    //SoBaseColor* myColors = new SoBaseColor;
    //myColors->rgb.setValues(0, 4, colors);
    //bgnd->addChild(myColors);

    // Define colors for the faces
    SoMaterial* myMaterials = new SoMaterial;
    myMaterials->diffuseColor.setValues(0, 4, colors);
    bgnd->addChild(myMaterials);
    SoMaterialBinding* myMaterialBinding = new SoMaterialBinding;
    myMaterialBinding->value = SoMaterialBinding::PER_VERTEX;
    bgnd->addChild(myMaterialBinding);
    SoEnvironment* environment = new SoEnvironment;
    environment->ambientIntensity = 1; // Set ambient light intensity
    bgnd->addChild(environment);
    SoFaceSet* myFaceSet = new SoFaceSet;
    myFaceSet->numVertices.set1Value(0, 4);  // One quad with 4 vertices
    bgnd->addChild(myFaceSet);
}


int
main(int argc, char** argv)
{
    SbBox3f boundingBox;
    int texW = 512;
    int texH = 512;
    QApplication app(argc, argv);
    
    // Initializes Quarter library (and implicitly also the Coin and Qt
    // libraries).
    Quarter::init();

    // Create a QuarterWidget for displaying a Coin scene graph
    QuarterWidget* viewer = new QuarterWidget;

    // Make a dead simple scene graph by using the Coin library, only
    // containing a single yellow cone under the scene graph root.
    SoSeparator* root = new SoSeparator;
    root->ref();

    SoFC3DEffects::initClass();

    SoSeparator* scene = new SoSeparator;
    SoPerspectiveCamera* pcam = new SoPerspectiveCamera;
    pcam->pointAt(SbVec3f(0, 0, 0));
    pcam->position = SbVec3f(0, 0, 8);
    scene->addChild(pcam);

    SoSeparator* object = new SoSeparator;
    CreateObject(object, boundingBox);
    scene->addChild(object);

    SoFC3DEffects* effects = new SoFC3DEffects;
    //effects->createScene();
    effects->setScene(object);
    scene->addChild(effects);

    // set a background
    SoSeparator* bgnd = new SoSeparator;
    CreateBackground(bgnd);

    //root->addChild(renderCallback);
    root->addChild(bgnd);
    root->addChild(scene);

    viewer->setSceneGraph(root);

    // make the viewer react to input events similar to the good old
    // ExaminerViewer
    viewer->setNavigationModeFile(QUrl("coin:///scxml/navigation/examiner.xml"));
    viewer->getSoEventManager()->setCamera(pcam);

    // Pop up the QuarterWidget
    viewer->show();
    // Loop until exit.
    app.exec();
    // Clean up resources.
    root->unref();
    delete viewer;

    Quarter::clean();

    return 0;
}

