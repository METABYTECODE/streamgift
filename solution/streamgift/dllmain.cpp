#include <windows.h>
#include "jvm/jni.h"
#include "classes_tsunami_v4_mc_1_7_10.h"
#include "classes_tsunami_v4_mc_1_8_9.h"
#include "classes_tsunami_v4_mc_1_12_2.h"
#include "loader.h"

enum cheat_version {
	unknown,
	mc_1_7_10,
	mc_1_8_9,
	mc_1_12_2
};

cheat_version version = unknown;

void check_window(const HWND hwnd) {
	wchar_t title[256];
	GetWindowText(hwnd, title, 256);
	if (!lstrcmpW(title, L"StreamCraft | Techno")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | Pixelmon")) version = mc_1_12_2;
	if (!lstrcmpW(title, L"StreamCraft | Techno")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | JediCraft")) version = mc_1_8_9;
	if (!lstrcmpW(title, L"StreamCraft | CivCraft")) version = mc_1_8_9;
	if (!lstrcmpW(title, L"StreamCraft | MiniGames")) version = unknown;
	if (!lstrcmpW(title, L"StreamCraft | TechnoMagicSky")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | TechnoMagic")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | RPG")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | Magic")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | Galactic")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | NanoTech")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | SandBox")) version = mc_1_7_10;
	if (!lstrcmpW(title, L"StreamCraft | Z.O.N.A.")) version = mc_1_7_10;
}

struct cheatpack_info {
	const unsigned char* classdata;
	const size_t* class_sizes;
	const size_t class_count;
	const char* mainclass_name;

	cheatpack_info(const unsigned char* classdata, const size_t* class_sizes, const size_t class_count, const char* mainclass_name) :
		classdata(classdata), class_sizes(class_sizes), class_count(class_count), mainclass_name(mainclass_name) { }
};

cheatpack_info get_cheatpack() {
	switch (version) {
		case unknown: 
			return cheatpack_info(nullptr, nullptr, 0, nullptr);
		case mc_1_7_10: 
			return cheatpack_info(classes_tsunami_v4_mc_1_7_10, classdata_sizes_tsunami_v4_mc_1_7_10, 
				sizeof classdata_sizes_tsunami_v4_mc_1_7_10 / sizeof classdata_sizes_tsunami_v4_mc_1_7_10[0],
				"com.sun.proxy.$Proxy99$Main");
		case mc_1_8_9: 
			return cheatpack_info(classes_tsunami_v4_mc_1_8_9, classdata_sizes_tsunami_v4_mc_1_8_9,
				sizeof classdata_sizes_tsunami_v4_mc_1_8_9 / sizeof classdata_sizes_tsunami_v4_mc_1_8_9[0],
				"com.sun.proxy.$Proxy99$Main");
		case mc_1_12_2: 
			return cheatpack_info(classes_tsunami_v4_mc_1_12_2, classdata_sizes_tsunami_v4_mc_1_12_2,
				sizeof classdata_sizes_tsunami_v4_mc_1_12_2 / sizeof classdata_sizes_tsunami_v4_mc_1_12_2[0],
				"com.sun.proxy.$Proxy99$Main");
		default: 
			return cheatpack_info(nullptr, nullptr, 0, nullptr);
	}
}

BOOL CALLBACK windows_callback(const HWND hwnd, LPARAM) {
	DWORD process_id = 0;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (process_id != GetProcessId(GetCurrentProcess()))
		return TRUE;
	check_window(hwnd);
	return version == unknown;
}

DWORD WINAPI main_thread(LPVOID) {
	EnumWindows(windows_callback, 0);

	if (version == unknown) {
		MessageBox(nullptr, L"Ошибка определения версии StreamCraft! Проверьте процесс в который Вы производите инжект!", L"ELoader by radioegor146", MB_OK | MB_ICONERROR);
		return 0;
	}

	const auto jvmdll_instance = GetModuleHandle(L"jvm.dll");
	if (!jvmdll_instance) {
		MessageBox(nullptr, L"Ошибка #1", L"ELoader by radioegor146", MB_OK | MB_ICONERROR);
		return 0;
	}

	typedef jint(JNICALL * get_created_javavms_type)(JavaVM **, jsize, jsize *);

	const auto get_created_javavms_proc = get_created_javavms_type(GetProcAddress(jvmdll_instance, "JNI_GetCreatedJavaVMs"));
	if (!get_created_javavms_proc) {
		MessageBox(nullptr, L"Ошибка #2", L"ELoader by radioegor146", MB_OK | MB_ICONERROR);
		return 0;
	}

	auto vms_count = 1;
	JavaVM* vm = nullptr;
	get_created_javavms_proc(&vm, vms_count, &vms_count);

	if (vms_count == 0)	{
		MessageBox(nullptr, L"Ошибка #3", L"ELoader by radioegor146", MB_OK | MB_ICONERROR);
		return 0;
	}

	JNIEnv* jni_env = nullptr;
	vm->AttachCurrentThread(reinterpret_cast<void **>(&jni_env), nullptr);
	vm->GetEnv(reinterpret_cast<void **>(&jni_env), JNI_VERSION_1_8);

	if (!jni_env) {
		MessageBox(nullptr, L"Ошибка #4.1", L"ELoader by radioegor146", MB_OK | MB_ICONERROR);
		vm->DetachCurrentThread();
		return 0;
	}

	const auto loader_clazz = jni_env->DefineClass(nullptr, nullptr, reinterpret_cast<jbyte*>(loader_class_data), sizeof loader_class_data);
	if (!loader_clazz) {
		MessageBox(nullptr, L"Ошибка #4.2", L"ELoader by radioegor146", MB_OK | MB_ICONERROR);
		vm->DetachCurrentThread();
		return 0;
	}

	const auto cheatpack = get_cheatpack();

	const auto classdatas_array = static_cast<jobjectArray>(
		jni_env->CallStaticObjectMethod(loader_clazz,
		    jni_env->GetStaticMethodID(
			    loader_clazz, "getByteArray", "(I)[[B"),
		    jint(cheatpack.class_count)));

	auto current_classdata_pointer = reinterpret_cast<const jbyte*>(cheatpack.classdata);
	for (size_t j = 0; j < cheatpack.class_count; j++) {
		const auto current_class_size = jsize(cheatpack.class_sizes[j]);
		const auto classdata_array = jni_env->NewByteArray(current_class_size);
		jni_env->SetByteArrayRegion(classdata_array, 0, current_class_size, current_classdata_pointer);
		current_classdata_pointer += current_class_size;
		jni_env->SetObjectArrayElement(classdatas_array, jsize(j), classdata_array);
	}

	const auto inject_result = jni_env->CallStaticIntMethod(loader_clazz, 
		jni_env->GetStaticMethodID(loader_clazz, "injectCP", "([[BLjava/lang/String;)I"), 
		classdatas_array, jni_env->NewStringUTF(cheatpack.mainclass_name));
	if (inject_result) {
		MessageBox(nullptr, L"Ошибка #4.3", L"ELoader by radioegor146", MB_OK | MB_ICONERROR);
		vm->DetachCurrentThread();
		return 0;
	}
	MessageBox(nullptr, L"Успешно инжектнуто! Приятной игры!", L"ELoader by radioegor146", MB_OK | MB_ICONINFORMATION);

	vm->DetachCurrentThread();
	return 0;
}

BOOL APIENTRY dll_main(HINSTANCE, const DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) 
		CreateThread(nullptr, 0, &main_thread, nullptr, 0, nullptr);
	return TRUE;
}
