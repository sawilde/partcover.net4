namespace PartViewer.Model
{
    public enum ActionKeyKind
    {
        KeyDown,
        KeyUp,
        KeyPress
    }

    public delegate void KeyActionHandler(ActionKeyKind kind);
}
