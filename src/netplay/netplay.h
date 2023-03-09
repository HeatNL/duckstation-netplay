#pragma once

#ifndef _NETPLAY_H
#define _NETPLAY_H

#include <ggponet.h>
#include <stdint.h>
#include <string>

namespace Netplay {

struct Input
{
  uint32_t button_data;
};

struct LoopTimer
{
public:
  void Init(uint32_t fps, uint32_t frames_to_spread_wait);
  void OnGGPOTimeSyncEvent(float frames_ahead);
  // Call every loop, to get the amount of time the current iteration of gameloop should take
  int32_t UsToWaitThisLoop();

private:
  float m_last_advantage = 0.0f;
  int32_t m_us_per_game_loop = 0;
  int32_t m_us_ahead = 0;
  int32_t m_us_extra_to_wait = 0;
  int32_t m_frames_to_spread_wait = 0;
  int32_t m_wait_count = 0;
};

class Session
{
public:
  Session();
  ~Session();
  // l = local, r = remote
  int32_t Start(int32_t lhandle, uint16_t lport, std::string& raddr, uint16_t rport, int32_t ldelay, uint32_t pred,
                GGPOSessionCallbacks* cb);

  void Close();
  bool IsActive();
  void RunIdle();

  void AdvanceFrame();
  int32_t CurrentFrame();

  std::string& GetGamePath();
  void SetGamePath(std::string& path);

  void SendMsg(const char* msg);

  GGPOErrorCode SyncInput(Netplay::Input inputs[2], int* disconnect_flags);
  GGPOErrorCode AddLocalInput(Netplay::Input input);
  GGPONetworkStats& GetNetStats(int32_t handle);
  GGPOPlayerHandle GetLocalHandle();

  Netplay::LoopTimer* GetTimer();

private:
  Netplay::LoopTimer m_timer;
  std::string m_game_path;
  uint32_t m_max_pred = 0;

  GGPOPlayerHandle m_local_handle = GGPO_INVALID_HANDLE;
  GGPONetworkStats m_last_net_stats;
  GGPOSession* p_ggpo = nullptr;
};

} // namespace Netplay

#endif // !_NETPLAY_H
