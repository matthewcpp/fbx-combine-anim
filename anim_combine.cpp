#include <iostream>

#include <fbxsdk.h>

FbxScene* CreateSceneFromFile(const char* fileName, FbxManager* fbxManager);
void CreateAnimations(FbxScene* destScene, FbxScene* srcScene);

int main(int argc, char** argv) {

	FbxManager* fbxManager = FbxManager::Create();
	FbxScene* masterScene = nullptr;

	FbxTime::SetGlobalTimeMode(FbxTime::eFrames60);
	

	for (int i = 1; i < argc; i++) {
		std::cout << "Loading Scene: " << argv[i] << std::endl;
		FbxScene* scene = CreateSceneFromFile(argv[i], fbxManager);

		if (!masterScene) {
			masterScene = scene;
		}
		else {
			CreateAnimations(masterScene, scene);
		}
	}

	FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	ioSettings->SetBoolProp(EXP_FBX_EMBEDDED, true);

	FbxExporter* exporter = FbxExporter::Create(fbxManager, "");
	exporter->Initialize("test_out.fbx", -1, ioSettings);
	exporter->SetFileExportVersion(FBX_2014_00_COMPATIBLE);
	exporter->Export(masterScene);
	exporter->Destroy();
		
	fbxManager->Destroy();

	return 0;
}

FbxScene* CreateSceneFromFile(const char* fileName, FbxManager* fbxManager) {
	FbxScene* scene = FbxScene::Create(fbxManager, fileName);

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");
	const bool result = importer->Initialize(fileName, -1, fbxManager->GetIOSettings());

	if (result) {
		importer->Import(scene);
	}
	else {
		FbxString error = importer->GetStatus().GetErrorString();
		FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", error.Buffer());
	}

	importer->Destroy();

	return scene;
}

void CopyKeys(FbxAnimCurve* destCurve, FbxAnimCurve* srcCurve) {
	destCurve->KeyModifyBegin();

	int srcKeyCount = srcCurve->KeyGetCount();

	for (int i = 0; i < srcKeyCount; i++) {
		int keyIndex = destCurve->KeyAdd(srcCurve->KeyGetTime(i));

		destCurve->KeySetInterpolation(keyIndex, srcCurve->KeyGetInterpolation(i));
		destCurve->KeySetValue(keyIndex, srcCurve->KeyGetValue(i));
	}

	destCurve->KeyModifyEnd();
}

void CreateAnimCurvesForProperty(FbxProperty& destProperty, FbxAnimLayer* destLayer, FbxProperty& srcProperty, FbxAnimLayer* srcLayer) {
	FbxAnimCurve* srcCurveX = srcProperty.GetCurve(srcLayer, FBXSDK_CURVENODE_COMPONENT_X, false);
	FbxAnimCurve* srcCurveY = srcProperty.GetCurve(srcLayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
	FbxAnimCurve* srcCurveZ = srcProperty.GetCurve(srcLayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

	if (srcCurveX) {
		FbxAnimCurve* destCurveX = destProperty.GetCurve(destLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* destCurveY = destProperty.GetCurve(destLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* destCurveZ = destProperty.GetCurve(destLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		CopyKeys(destCurveX, srcCurveX);
		CopyKeys(destCurveY, srcCurveY);
		CopyKeys(destCurveZ, srcCurveZ);
	}
}

void CreateCurves(FbxAnimLayer* destlayer, FbxNode* destNode, FbxAnimLayer* srclayer, FbxNode* srcNode) {
	CreateAnimCurvesForProperty(destNode->LclTranslation, destlayer, srcNode->LclTranslation, srclayer);
	CreateAnimCurvesForProperty(destNode->LclRotation, destlayer, srcNode->LclRotation, srclayer);
	CreateAnimCurvesForProperty(destNode->LclScaling, destlayer, srcNode->LclScaling, srclayer);

	int childCount = srcNode->GetChildCount();
	for (int i = 0; i < childCount; i++) {
		CreateCurves(destlayer, destNode->GetChild(i), srclayer, srcNode->GetChild(i));
	}
	
}

void CreateAnimations(FbxScene* destScene, FbxScene* srcScene) {
	int srcStackCount = srcScene->GetSrcObjectCount<FbxAnimStack>();
	for (int i = 0; i < srcStackCount; i++) {
		FbxAnimStack* srcStack = srcScene->GetSrcObject<FbxAnimStack>(i);
		FbxAnimLayer* srcLayer = srcStack->GetSrcObject<FbxAnimLayer>(0);

		destScene->CreateAnimStack(srcStack->GetName());
		FbxAnimStack* destStack = destScene->GetSrcObject<FbxAnimStack>(destScene->GetSrcObjectCount<FbxAnimStack>() -1);

		FbxAnimLayer* destLayer = FbxAnimLayer::Create(destScene, srcLayer->GetName());
		destStack->AddMember(destLayer);

		CreateCurves(destLayer, destScene->GetRootNode(), srcLayer, srcScene->GetRootNode());

		destStack->LocalStop.Set(srcStack->LocalStop.Get());
	}	
}
