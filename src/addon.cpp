#ifdef __GNUC__
#define __cdecl
#endif

#include <kodi/addon-instance/Visualization.h>
#include <kodi/General.h>

#include <vsx_version.h>
#include <vsx_platform.h>
#include <vsx_manager.h>
#include <vsx_gl_state.h>


#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

// TODO: configure setting.
// https://github.com/vovoid/vsxu/blob/master/engine_audiovisual/src/vsx_manager.cpp
// https://github.com/vovoid/vsxu/blob/master/engine_audiovisual/src/vsx_statelist.h

class CVisualizationVSXU
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceVisualization
{
public:
  CVisualizationVSXU();
  virtual ~CVisualizationVSXU();

  virtual void Render() override;
  virtual void AudioData(const float* audioData, int audioDataLength, float *freqData, int freqDataLength) override;
  virtual bool GetPresets(std::vector<std::string>& presets) override;
  virtual bool LoadPreset(int select) override;
  virtual bool PrevPreset();
  virtual bool NextPreset();
  virtual bool LockPreset(bool lockUnlock);
  virtual unsigned int GetPreset() override;

private:
  vsx_manager_abs* m_manager;
  std::vector<std::string> m_presets;
};

CVisualizationVSXU::CVisualizationVSXU()
{
  std::string path = kodi::GetAddonPath() + "/resources";

  // create a new manager
  m_manager = manager_factory();
  m_manager->set_option_preload_all(false);

  m_manager->init("/usr/share/vsxu", "media_player");
  m_manager->add_visual_path(path.c_str());
  m_presets = m_manager->get_visual_filenames();

  // strip off dir names - if there are duped presets this will misbehave.
  for (size_t i=0;i<m_presets.size();++i)
  {
    size_t sit = m_presets[i].rfind('/');
    size_t dit = m_presets[i].rfind('.');
    m_presets[i] = m_presets[i].substr(sit+1, dit-sit-1);
  }

  vsx_gl_state::get_instance()->viewport_set(0, 0, Width(), Height());
}

CVisualizationVSXU::~CVisualizationVSXU()
{
  // stop vsxu nicely (unloads textures and frees memory)
  if (m_manager)
    m_manager->stop();

  // call manager factory to destruct our manager object
  if (m_manager)
    manager_destroy(m_manager);
}

void CVisualizationVSXU::AudioData(const float *pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength)
{
  m_manager->set_sound_wave(const_cast<float*>(pAudioData));
  m_manager->set_sound_freq(const_cast<float*>(pFreqData));
}

void CVisualizationVSXU::Render()
{
  m_manager->render();
}

bool CVisualizationVSXU::LoadPreset(int select)
{
  m_manager->pick_visual(select);
  return true;
}

bool CVisualizationVSXU::GetPresets(std::vector<std::string>& presets)
{
  for (size_t i = 0; i < m_presets.size(); ++i)
    presets.push_back(m_presets[i]);

  return (m_presets.size() > 0);
}

bool CVisualizationVSXU::PrevPreset()
{
  m_manager->prev_visual();
  return true;
}

bool CVisualizationVSXU::NextPreset()
{ 
  m_manager->next_visual();
  return true;
}

bool CVisualizationVSXU::LockPreset(bool lockUnlock)
{
  m_manager->toggle_randomizer();
  return true;
}

class FindSubString
{
  public:
    FindSubString(const std::string& b) : m_b(b) {}

    bool operator()(const std::string& a)
    {
      return m_b.find(a) != std::string::npos;
    }

    std::string m_b;
};

unsigned int CVisualizationVSXU::GetPreset()
{
  std::string current = m_manager->get_meta_visual_filename();

  std::vector<std::string>::const_iterator it = std::find_if(m_presets.begin(),
                                                             m_presets.end(),
                                                             FindSubString(current + ".vsx"));

  return it-m_presets.begin();
}

ADDONCREATOR(CVisualizationVSXU) // Don't touch this!
