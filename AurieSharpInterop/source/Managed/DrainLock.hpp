#pragma once
#pragma once

#pragma once

public ref class DrainableMutex
{
private:
    System::Object^ m_Sync;
    bool m_Accepting;
    int m_ActiveCount;
    System::Threading::ManualResetEventSlim^ m_AllReleased;

public:
    DrainableMutex()
    {
        m_Sync = gcnew System::Object();
        m_Accepting = true;
        m_ActiveCount = 0;
        m_AllReleased = gcnew System::Threading::ManualResetEventSlim(true); // initially signaled
    }

    void Wait()
    {
        while (true)
        {
            System::Threading::Monitor::Enter(m_Sync);
            try
            {
                if (m_Accepting)
                {
                    m_ActiveCount++;
                    m_AllReleased->Reset();
                    return; // acquired successfully
                }
                // else, not accepting yet; fall through and wait
            }
            finally
            {
                System::Threading::Monitor::Exit(m_Sync);
            }
        }
    }

    void Release()
    {
        System::Threading::Monitor::Enter(m_Sync);
        try
        {
            m_ActiveCount--;
            if (m_ActiveCount == 0)
                m_AllReleased->Set();
        }
        finally
        {
            System::Threading::Monitor::Exit(m_Sync);
        }
    }

    // Prevent new Wait() calls and block until all existing ones are Released
    void StopNewWaitsAndDrain()
    {
        System::Threading::Monitor::Enter(m_Sync);
        try
        {
            m_Accepting = false;
            if (m_ActiveCount == 0)
                return; // already drained
        }
        finally
        {
            System::Threading::Monitor::Exit(m_Sync);
        }

        m_AllReleased->Wait(); // block until drained
    }

    // Re-enable Wait() calls
    void Resume()
    {
        System::Threading::Monitor::Enter(m_Sync);
        try
        {
            if (m_ActiveCount != 0)
                throw gcnew System::InvalidOperationException("Cannot resume while active waits exist");

            m_Accepting = true;
            m_AllReleased->Set();
        }
        finally
        {
            System::Threading::Monitor::Exit(m_Sync);
        }
    }
};
