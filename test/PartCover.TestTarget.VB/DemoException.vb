Imports NUnit.Framework


Public Class DemoException
    Inherits Exception

    Public Sub New(ByVal x As Integer)
        x = x
    End Sub

    Public Property X As Integer

End Class
