import { serve } from "https://deno.land/std@0.168.0/http/server.ts";

const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Headers":
    "authorization, x-client-info, apikey, content-type, x-supabase-client-platform, x-supabase-client-platform-version, x-supabase-client-runtime, x-supabase-client-runtime-version",
};

serve(async (req) => {
  if (req.method === "OPTIONS") {
    return new Response(null, { headers: corsHeaders });
  }

  try {
    const { code, fileName } = await req.json();

    if (!code) {
      return new Response(
        JSON.stringify({ error: "Missing code" }),
        { status: 400, headers: { ...corsHeaders, "Content-Type": "application/json" } }
      );
    }

    const GEMINI_API_KEY = Deno.env.get("GEMINI_API_KEY");
    if (!GEMINI_API_KEY) {
      throw new Error("GEMINI_API_KEY is not configured");
    }

    const systemPrompt = `Você é o núcleo de execução da linguagem PoolScript. Sua única função é traduzir a intenção do usuário em código Python executável e otimizado.

REGRAS CRÍTICAS:
Responda APENAS com código Python puro. Não use explicações, não use blocos de Markdown, não escreva comentários no código.
Se a intenção for ambígua ou faltar algo necessário, deixe o Python mandar o erro para o terminal do usuário.
Se o usuário mencionar arquivos locais, use caminhos relativos compatíveis com Windows.
Adicione tratamento de erros try-except básico para reportar falhas de forma limpa.`;

    const response = await fetch(
      `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=${GEMINI_API_KEY}`,
      {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          contents: [
            {
              parts: [
                {
                  text: `${systemPrompt}\n\nArquivo: ${fileName}\nCódigo PoolScript:\n${code}`
                }
              ]
            }
          ],
          generationConfig: {
            temperature: 0.1,
            maxOutputTokens: 2048,
          }
        }),
      }
    );

    if (!response.ok) {
      if (response.status === 429) {
        return new Response(
          JSON.stringify({ error: "Rate limit. Tente novamente em instantes." }),
          { status: 429, headers: { ...corsHeaders, "Content-Type": "application/json" } }
        );
      }
      const errText = await response.text();
      console.error("Gemini error:", response.status, errText);
      throw new Error("Gemini API error");
    }

    const aiData = await response.json();
    let transpiledCode = aiData.candidates?.[0]?.content?.parts?.[0]?.text || "";

    // Remove blocos markdown se a IA ignorar as regras
    const codeBlockMatch = transpiledCode.match(/```(?:python)?\s*([\s\S]*?)```/);
    if (codeBlockMatch) {
      transpiledCode = codeBlockMatch[1].trim();
    }

    return new Response(
      JSON.stringify({ transpiledCode: transpiledCode.trim() }),
      { headers: { ...corsHeaders, "Content-Type": "application/json" } }
    );

  } catch (e) {
    console.error("transpile error:", e);
    return new Response(
      JSON.stringify({ error: e instanceof Error ? e.message : "Unknown error" }),
      { status: 500, headers: { ...corsHeaders, "Content-Type": "application/json" } }
    );
  }
});
