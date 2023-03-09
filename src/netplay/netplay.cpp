#include "netplay.h"

Netplay::Session::Session() = default;

Netplay::Session::~Session()
{
  Close();
}

int32_t Netplay::Session::Start(int32_t lhandle, uint16_t lport, std::string& raddr, uint16_t rport, int32_t ldelay,
                                uint32_t pred, GGPOSessionCallbacks* cb)
{
  m_max_pred = pred;
  GGPOErrorCode result;

  result = ggpo_start_session(&p_ggpo, cb, "Duckstation-Netplay", 2, sizeof(Netplay::Input), lport, m_max_pred);

  ggpo_set_disconnect_timeout(p_ggpo, 3000);
  ggpo_set_disconnect_notify_start(p_ggpo, 1000);

  for (int i = 1; i <= 2; i++)
  {
    GGPOPlayer player = {};
    GGPOPlayerHandle handle = 0;

    player.size = sizeof(GGPOPlayer);
    player.player_num = i;

    if (lhandle == i)
    {
      player.type = GGPOPlayerType::GGPO_PLAYERTYPE_LOCAL;
      result = ggpo_add_player(p_ggpo, &player, &handle);
      m_local_handle = handle;
    }
    else
    {
      player.type = GGPOPlayerType::GGPO_PLAYERTYPE_REMOTE;
#ifdef _WIN32
      strcpy_s(player.u.remote.ip_address, raddr.c_str());
#else
      strcpy(player.u.remote.ip_address, raddr.c_str());
#endif
      player.u.remote.port = rport;
      result = ggpo_add_player(p_ggpo, &player, &handle);
    }
  }
  ggpo_set_frame_delay(p_ggpo, m_local_handle, ldelay);

  return result;
}

void Netplay::Session::Close()
{
  ggpo_close_session(p_ggpo);
  p_ggpo = nullptr;
  m_local_handle = GGPO_INVALID_HANDLE;
  m_max_pred = 0;
}

bool Netplay::Session::IsActive()
{
  return p_ggpo != nullptr;
}

void Netplay::Session::RunIdle()
{
  ggpo_idle(p_ggpo);
}

void Netplay::Session::AdvanceFrame()
{
  ggpo_advance_frame(p_ggpo, 0);
}

int32_t Netplay::Session::CurrentFrame()
{
  int32_t frame;
  ggpo_get_current_frame(p_ggpo, frame);
  return frame;
}

std::string& Netplay::Session::GetGamePath()
{
  return m_game_path;
}

void Netplay::Session::SetGamePath(std::string& path)
{
  m_game_path = path;
}

void Netplay::Session::SendMsg(const char* msg)
{
  ggpo_client_chat(p_ggpo, msg);
}

GGPOErrorCode Netplay::Session::SyncInput(Netplay::Input inputs[2], int* disconnect_flags)
{
  return ggpo_synchronize_input(p_ggpo, inputs, sizeof(Netplay::Input) * 2, disconnect_flags);
}

GGPOErrorCode Netplay::Session::AddLocalInput(Netplay::Input input)
{
  return ggpo_add_local_input(p_ggpo, m_local_handle, &input, sizeof(Netplay::Input));
}

GGPONetworkStats& Netplay::Session::GetNetStats(int32_t handle)
{
  ggpo_get_network_stats(p_ggpo, handle, &m_last_net_stats);
  return m_last_net_stats;
}

GGPOPlayerHandle Netplay::Session::GetLocalHandle()
{
  return m_local_handle;
}

Netplay::LoopTimer* Netplay::Session::GetTimer()
{
  return &m_timer;
}

void Netplay::LoopTimer::Init(uint32_t fps, uint32_t frames_to_spread_wait)
{
  m_us_per_game_loop = 1000000 / fps;
  m_us_ahead = 0;
  m_us_extra_to_wait = 0;
  m_frames_to_spread_wait = frames_to_spread_wait;
  m_last_advantage = 0.0f;
}

void Netplay::LoopTimer::OnGGPOTimeSyncEvent(float frames_ahead)
{
  m_last_advantage = (1000.0f * frames_ahead / 60.0f);
  m_last_advantage /= 2;
  if (m_last_advantage < 0)
  {
    int t = 0;
    t++;
  }
  m_us_extra_to_wait = (int)(m_last_advantage * 1000);
  if (m_us_extra_to_wait)
  {
    m_us_extra_to_wait /= m_frames_to_spread_wait;
    m_wait_count = m_frames_to_spread_wait;
  }
}

int32_t Netplay::LoopTimer::UsToWaitThisLoop()
{
  int32_t timetoWait = m_us_per_game_loop;
  if (m_wait_count)
  {
    timetoWait += m_us_extra_to_wait;
    m_wait_count--;
    if (!m_wait_count)
      m_us_extra_to_wait = 0;
  }
  return timetoWait;
}
