
namespace signalr
{
    enum class trace_level
    {
        none,
        messages,
        events,
        state_changes,
        all = messages | events | state_changes
    };
}