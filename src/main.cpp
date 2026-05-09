#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include <voicevox_core.h>

namespace
{
constexpr VoicevoxStyleId kStyleId = 14;

void PrintUsage(const char *programName)
{
    std::cerr << "Usage: " << programName << " <onnxruntime.dylib> <open_jtalk_dict_dir> <voice_model.vvm> <output.wav> \"text to synthesize\"\n";
}

bool WriteFile(const std::string &path, const uint8_t *data, size_t size)
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open output file: " << path << "\n";
        return false;
    }

    file.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
    if (!file)
    {
        std::cerr << "Failed to write output file: " << path << "\n";
        return false;
    }

    return true;
}

bool CheckVoicevox(VoicevoxResultCode result, const std::string &message)
{
    if (result == VOICEVOX_RESULT_OK)
    {
        return true;
    }

    std::cerr << message << ": " << voicevox_error_result_to_message(result) << "\n";
    return false;
}
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    const std::string onnxruntimePath = argv[1];
    const std::string openJtalkDictPath = argv[2];
    const std::string voiceModelPath = argv[3];
    const std::string outputPath = argv[4];
    const std::string text = argv[5];

    const VoicevoxOnnxruntime *onnxruntime = nullptr;
    VoicevoxLoadOnnxruntimeOptions loadOptions = voicevox_make_default_load_onnxruntime_options();
    loadOptions.filename = onnxruntimePath.c_str();
    if (!CheckVoicevox(voicevox_onnxruntime_load_once(loadOptions, &onnxruntime), "Failed to load ONNX Runtime"))
    {
        return 1;
    }

    OpenJtalkRc *openJtalk = nullptr;
    if (!CheckVoicevox(voicevox_open_jtalk_rc_new(openJtalkDictPath.c_str(), &openJtalk), "Failed to load OpenJTalk dictionary"))
    {
        return 1;
    }

    VoicevoxSynthesizer *synthesizer = nullptr;
    const VoicevoxInitializeOptions initializeOptions = voicevox_make_default_initialize_options();
    if (!CheckVoicevox(voicevox_synthesizer_new(onnxruntime, openJtalk, initializeOptions, &synthesizer), "Failed to create synthesizer"))
    {
        voicevox_open_jtalk_rc_delete(openJtalk);
        return 1;
    }
    voicevox_open_jtalk_rc_delete(openJtalk);

    VoicevoxVoiceModelFile *voiceModel = nullptr;
    if (!CheckVoicevox(voicevox_voice_model_file_open(voiceModelPath.c_str(), &voiceModel), "Failed to open voice model"))
    {
        voicevox_synthesizer_delete(synthesizer);
        return 1;
    }

    if (!CheckVoicevox(voicevox_synthesizer_load_voice_model(synthesizer, voiceModel), "Failed to load voice model"))
    {
        voicevox_voice_model_file_delete(voiceModel);
        voicevox_synthesizer_delete(synthesizer);
        return 1;
    }
    voicevox_voice_model_file_delete(voiceModel);

    uintptr_t wavSize = 0;
    uint8_t *wav = nullptr;
    const VoicevoxTtsOptions ttsOptions = voicevox_make_default_tts_options();
    const VoicevoxResultCode synthesisResult = voicevox_synthesizer_tts(
        synthesizer,
        text.c_str(),
        kStyleId,
        ttsOptions,
        &wavSize,
        &wav);

    if (!CheckVoicevox(synthesisResult, "Failed to synthesize text"))
    {
        voicevox_synthesizer_delete(synthesizer);
        return 1;
    }

    const bool wroteFile = WriteFile(outputPath, wav, wavSize);
    voicevox_wav_free(wav);
    voicevox_synthesizer_delete(synthesizer);

    if (!wroteFile)
    {
        return 1;
    }

    std::cout << "Wrote " << outputPath << "\n";
    return 0;
}
