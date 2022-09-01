#include <iostream>
#include <string>
#include <filesystem>

#include <fbxsdk.h>

fbxsdk::FbxScene* CreateSceneFromFile(const char* fileName, fbxsdk::FbxManager* fbxManager);
bool ExportScene(fbxsdk::FbxScene* scene, fbxsdk::FbxManager* fbxManager, const char* path);
void CreateAnimation(fbxsdk::FbxScene* destScene, fbxsdk::FbxScene* srcScene, std::string const & name);

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cout << "Usage: ./fbx-combine-anim <input_directory_path> <output_file_path>" << std::endl;
		return 1;
	}
	

	auto* fbxManager = fbxsdk::FbxManager::Create();
	fbxsdk::FbxScene* masterScene = nullptr;

	fbxsdk::FbxTime::SetGlobalTimeMode(fbxsdk::FbxTime::eFrames30);

	int i = 0;
	for (auto& p : std::filesystem::directory_iterator(argv[1])){
		if (std::filesystem::is_regular_file(p)) {
			auto filePath = p.path();

			std::string pathStr = filePath.string();
			std::string animName = filePath.stem().string();

			auto* scene = CreateSceneFromFile(pathStr.c_str(), fbxManager);

			if (!masterScene) {
				masterScene = scene;
				auto* animStack = masterScene->GetSrcObject<fbxsdk::FbxAnimStack>(0);
				animStack->SetName(animName.c_str());
			}
			else {
				CreateAnimation(masterScene, scene, animName);
			}
		}
	}

	std::string outputPath = argv[2];
	ExportScene(masterScene, fbxManager, outputPath.c_str());
	
	fbxManager->Destroy();

	return 0;
}

fbxsdk::FbxScene* CreateSceneFromFile(const char* fileName, fbxsdk::FbxManager* fbxManager) {
	std::cout << "Processing: " << fileName << std::endl;
	auto* scene = FbxScene::Create(fbxManager, fileName);

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");
	bool result = importer->Initialize(fileName, -1, fbxManager->GetIOSettings());

	if (result) {
		result = importer->Import(scene);
	}
	else {
		FbxString error = importer->GetStatus().GetErrorString();
		std::cout << "FBX Import Failed: " << error.Buffer() << std::endl;
	}

	importer->Destroy();

	return scene;
}

bool ExportScene(fbxsdk::FbxScene* scene, fbxsdk::FbxManager* fbxManager, const char* path) {
	std::cout << "Writing: " << path << std::endl;

	FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	ioSettings->SetBoolProp(EXP_FBX_EMBEDDED, true);

	FbxExporter* exporter = FbxExporter::Create(fbxManager, "");
	bool result = exporter->Initialize(path, -1, ioSettings);

	if (result) {
		exporter->SetFileExportVersion(FBX_2014_00_COMPATIBLE);
		result = exporter->Export(scene);
	}

	if (!result) {
		FbxString error = exporter->GetStatus().GetErrorString();
		std::cout << "FBX Export Failed: " << error.Buffer() << std::endl;
	}

	exporter->Destroy();

	return result;
}

void CopyKeys(fbxsdk::FbxAnimCurve* destCurve, fbxsdk::FbxAnimCurve* srcCurve) {
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
	auto* srcCurveX = srcProperty.GetCurve(srcLayer, FBXSDK_CURVENODE_COMPONENT_X, false);
	auto* srcCurveY = srcProperty.GetCurve(srcLayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
	auto* srcCurveZ = srcProperty.GetCurve(srcLayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

	if (srcCurveX) {
		auto* destCurveX = destProperty.GetCurve(destLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		auto* destCurveY = destProperty.GetCurve(destLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		auto* destCurveZ = destProperty.GetCurve(destLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		CopyKeys(destCurveX, srcCurveX);
		CopyKeys(destCurveY, srcCurveY);
		CopyKeys(destCurveZ, srcCurveZ);
	}
}

void CreateCurves(fbxsdk::FbxAnimLayer* destlayer, fbxsdk::FbxNode* destNode, fbxsdk::FbxAnimLayer* srclayer, fbxsdk::FbxNode* srcNode) {
	CreateAnimCurvesForProperty(destNode->LclTranslation, destlayer, srcNode->LclTranslation, srclayer);
	CreateAnimCurvesForProperty(destNode->LclRotation, destlayer, srcNode->LclRotation, srclayer);
	CreateAnimCurvesForProperty(destNode->LclScaling, destlayer, srcNode->LclScaling, srclayer);

	int childCount = srcNode->GetChildCount();
	for (int i = 0; i < childCount; i++) {
		CreateCurves(destlayer, destNode->GetChild(i), srclayer, srcNode->GetChild(i));
	}
	
}

void CreateAnimation(FbxScene* destScene, FbxScene* srcScene, std::string const & name) {
	auto* srcStack = srcScene->GetSrcObject<fbxsdk::FbxAnimStack>(0);
	auto* srcLayer = srcStack->GetSrcObject<fbxsdk::FbxAnimLayer>(0);

	destScene->CreateAnimStack(name.c_str());
	auto* destStack = destScene->GetSrcObject<fbxsdk::FbxAnimStack>(destScene->GetSrcObjectCount<fbxsdk::FbxAnimStack>() -1);

	auto* destLayer = fbxsdk::FbxAnimLayer::Create(destScene, srcLayer->GetName());
	destStack->AddMember(destLayer);

	CreateCurves(destLayer, destScene->GetRootNode(), srcLayer, srcScene->GetRootNode());

	destStack->LocalStop.Set(srcStack->LocalStop.Get());
}
